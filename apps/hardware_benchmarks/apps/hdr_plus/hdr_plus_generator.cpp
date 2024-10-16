
/*
 * An application that performs a simplified version of Google's HDR+ pipeline 
 * 
 */
#define T_SIZE 16
#define LOG_2_T_SIZE 4
#define G_T_SIZE 8
#include "Halide.h"

namespace {
int blockSize = 9;
//int blockSize = 5;
//int blockSize = 1;
constexpr int maxJ = 6;

using namespace Halide;
using namespace Halide::ConciseCasts;
using std::vector;
Var x("x"), y("y"), tx("tx"), ty("ty"), xy("xy"), xo("xo"), yo("yo"), xi("xi"), yi("yi"), c("c"), n("n"), k("k");

void fill_funcnames(vector<Func>& funcs, std::string name) {
  for (size_t i=0; i<funcs.size(); ++i) {
      funcs[i] = Func(name + "_" + std::to_string(i));
    }
  }

class HDRPlus : public Halide::Generator<HDRPlus> {
public:
    // FOR ALIGN
    GeneratorParam<int>     pyramid_levels{"pyramid_levels", 5};
    GeneratorParam<int>     total_downsample_factor{"total_downsample_factor", 16}; // 2^ pyramid_levels-1

    // Operate on raw bayer image: so 2 channels, plus 1 channel b/c receiving multiple images
    Input<Buffer<uint16_t>>  input{"input", 3};

    // Output alignPyramid[4]. 4 channels: tx, ty, xy, n
    Output<Buffer<uint8_t>> output{"output", 2};
    //Output<Buffer<uint8_t>> output{"output", 3};
    //Output<Buffer<int16_t>> output{"output", 4};

    // Multiply with maximum precision and make result 8.8 p
    Expr mul1(Expr a, Expr b) {
      return i16(( (i32(a)) * (i32(b)) ) >> 8);
    }


    Expr mul_1023(Expr a, Expr b) {
      return i16(( (i32(a)) * (i32(b)) ) >> 10);
    }


    Expr mul_1023_unsigned(Expr a, Expr b) {
      return u16(( (u32(a)) * (u32(b)) ) >> 10);
    }

    void generate() {
        // Algorithm
    
        /* 
         * ALIGN STEP
         */
        const int J = pyramid_levels;

      
       /* Func: clamped_input
        * dtype: u16
        * True range: [0, 1023]
        * Consumer(s): hw_input
        */
        Func clamped_input;
        clamped_input = Halide::BoundaryConditions::repeat_edge(input);


       /* Func: hw_input
        * dtype: u16
        * True range: [0, 1023]
        * Consumer(s): deinterleaved, val (in merge)
        */
        Func hw_input;
        hw_input(x, y, n) = clamped_input(x, y, n);

        Func hw_input_copy;
        hw_input_copy(x, y, n) = hw_input(x, y, n) + u16(0); 
  

        //hw_input.trace_stores();

       /* Func: deinterleaved
        * dtype: u16
        * True range: [0, 1023]
        * Consumer(s): gray
        */
        Func deinterleaved;
        deinterleaved(x, y, c, n) = select(c == 0, hw_input_copy(2 * x, 2 * y, n), (select(c == 1, hw_input_copy(2 * x + 1, 2 * y, n), 
                                            (select(c == 2, hw_input_copy(2 * x, 2 * y + 1, n), hw_input_copy(2 * x + 1, 2 * y + 1, n))))));


       // STEP 1: Convert to grayscale 
       /* Func: gray
        * dtype: u16
        * True range: [0, 1023]
        * Consumer(s): pyramids in align, dist_tile (in merge)
        */
        Func gray;   
        gray(x, y, n) = u16((deinterleaved(x, y, 1, n) + deinterleaved(x, y, 0, n) + deinterleaved(x, y, 3, n) + deinterleaved(x, y, 2, n)) >> 2); 

       
        //STEP 2: Downsample to form image pyramids 
        Expr initialGaussWidth[J];
        Expr initialGaussHeight[J];

        initialGaussWidth[0] = 128;
        initialGaussWidth[1] = 64;
        initialGaussWidth[2] = 32;
        initialGaussWidth[3] = 16;
        initialGaussWidth[4] = 8;
        
       
        initialGaussHeight[0] = 128;
        initialGaussHeight[1] = 64;
        initialGaussHeight[2] = 32;
        initialGaussHeight[3] = 16;
        initialGaussHeight[4] = 8;
  

       /* Func: gPyramid[J]
        * dtype: u16
        * True range: [0, 1023]
        * Consumer(s): dist & scores calculation (in align)
        */
        //Func gPyramid[J];
        vector<Func> gPyramid(J);
        gPyramid[0](x, y, n) = gray(x, y, n);

        gPyramid[1](x, y, n) = downsample_u16_hdr(gPyramid[0], 2, initialGaussWidth[0], initialGaussHeight[0])(x, y, n);
        gPyramid[2](x, y, n) = downsample_u16_hdr(gPyramid[1], 2, initialGaussWidth[1], initialGaussHeight[1])(x, y, n);
        gPyramid[3](x, y, n) = downsample_u16_hdr(gPyramid[2], 2, initialGaussWidth[2], initialGaussHeight[2])(x, y, n);
        gPyramid[4](x, y, n) = downsample_u16_hdr(gPyramid[3], 2, initialGaussWidth[3], initialGaussHeight[3])(x, y, n);
    

        //gPyramid[0].trace_stores();
        Func provisional_output;
        provisional_output(x, y, n) = gPyramid[4](x, y, n);

        //provisional_output(x, y, n) = hw_input_copy(x, y, n);
        //output(x, y, n) = u8(provisional_output(x, y, n));
        output(x, y) = u8(provisional_output(x, y, 0));
        //output.bound(n, 0, 3);
        // output.bound(x, 0, 8);
        // output.bound(y, 0, 8);

      // Schedule
      if (get_target().has_feature(Target::CoreIR)) {

      } else if (get_target().has_feature(Target::Clockwork)) {
    
        // CHANGE THIS 
        // const int output_x_size = 32;
        // const int output_y_size = 32;

        const int output_x_size = 8;
        const int output_y_size = 8;

        output.bound(x, 0, output_x_size);
        output.bound(y, 0, output_y_size);
        provisional_output.in().compute_root();

        provisional_output.in().tile(x, y, xo, yo, xi, yi, output_x_size, output_y_size)
          .reorder(xi, yi, xo, yo)
          .hw_accelerate(xi, xo);

        provisional_output.tile(x, y, xo, yo, xi, yi, output_x_size, output_y_size)
            .reorder(xi, yi, xo, yo);
        provisional_output.compute_at(provisional_output.in(), xo);
        provisional_output.store_in(MemoryType::GLB);

        // alignPyramid[4].compute_at(provisional_output, xo);

        gPyramid[0].compute_at(provisional_output, xo);
        gPyramid[1].compute_at(provisional_output, xo);
        gPyramid[2].compute_at(provisional_output, xo);
        gPyramid[3].compute_at(provisional_output, xo);
        gPyramid[4].compute_at(provisional_output, xo);

        gray.compute_at(provisional_output, xo); 
        hw_input_copy.compute_at(provisional_output, xo);
  
        hw_input.in().in().compute_at(provisional_output, xo); // represents the mem tile

        hw_input.in().compute_at(provisional_output.in(), xo); // represents the glb level
        hw_input.in().store_in(MemoryType::GLB);

        // hw_input.bound(x, 0, 256);
        // hw_input.bound(y, 0, 256);
        hw_input.bound(n, 0, 3);
        hw_input.compute_root()
          .accelerator_input();


      // SCHEDULE TO CPU
      } else {

        gPyramid[0].compute_root();
        gPyramid[1].compute_root();
        gPyramid[2].compute_root();

        hw_input.compute_root();
        
        
      }
    }
private:
    Func upsample_u16_size_2_for_alignment(Func f_in, Expr gauss_width, Expr gauss_height) {
      Var tx, ty, xy, n;
      using Halide::_;
      Func up, up_pre_shift, f_in_shift;

      up(tx, ty, xy, n) = f_in(tx/2, ty/2, xy, n);
      return up;
    }
    
     Func downsample_u16_hdr(Func f_in, Expr size, Expr gauss_width, Expr gauss_height) {
        Var x, y, n;
        using Halide::_;
        Func f, down_pre_shift, down;
    
        f(x, y, n) = f_in(x, y, n);
        // Expr x_index_0 = clamp(size*x-1, 0, gauss_width-1);
        // Expr x_index_1 = clamp(size*x, 0, gauss_width-1);
        // Expr x_index_2 = clamp(size*x+1, 0, gauss_width-1);
        // Expr x_index_3 = clamp(size*x+2, 0, gauss_width-1);

        // Expr x_index_0 = max(min(size*x-1, 0), gauss_width-1); 
        // Expr x_index_1 = max(min(size*x, 0), gauss_width-1); 
        // Expr x_index_2 = max(min(size*x+1, 0), gauss_width-1); 
        // Expr x_index_3 = max(min(size*x+2, 0), gauss_width-1); 

        Expr x_index_0 = size*x-1; 
        Expr x_index_1 = size*x; 
        Expr x_index_2 = size*x+1; 
        Expr x_index_3 = size*x+2; 
        
        // Expr y_index_0 = clamp(size*y-1, 0, gauss_height-1);
        // Expr y_index_1 = clamp(size*y, 0, gauss_height-1);
        // Expr y_index_2 = clamp(size*y+1, 0, gauss_height-1);
        // Expr y_index_3 = clamp(size*y+2, 0, gauss_height-1);

        Expr y_index_0 = size*y-1; 
        Expr y_index_1 = size*y; 
        Expr y_index_2 = size*y+1; 
        Expr y_index_3 = size*y+2; 

        // Expr y_index_0 = max(min(size*y-1, 0), gauss_height-1); 
        // Expr y_index_1 = max(min(size*y, 0), gauss_height-1); 
        // Expr y_index_2 = max(min(size*y+1, 0), gauss_height-1); 
        // Expr y_index_3 = max(min(size*y+2, 0), gauss_height-1); 


        // Try emulating clamping using conditionals? So if we're at the boundaries, use a smaller or different window than if
        // we are in the middle.

        // Note: If doing padding, turn the window into a sliding kernel and treat this operation like a convolution. 

      //  /* Func: down_pre_shift
      //   * dtype: u16
      //   * True range: [0, 65472] (worst case)
      //   * Consumer(s): down
      //   */
        // down_pre_shift(x, y, n) = (1) * f(clamp(size*x-1, 0, gauss_width-1), clamp(size*y-1, 0, gauss_height-1), n) + (3) * f(clamp(size*x-1, 0, gauss_width-1), clamp(size*y, 0, gauss_height-1), n) + (3) * f(clamp(size*x-1, 0, gauss_width-1), clamp(size*y+1, 0, gauss_height-1), n) + (1) * f(clamp(size*x-1, 0, gauss_width-1), clamp(size*y+2, 0, gauss_height-1), n) 
        //                 + (3) * f(clamp(size*x, 0, gauss_width-1), clamp(size*y-1, 0, gauss_height-1), n) + (9) * f(clamp(size*x, 0, gauss_width-1), clamp(size*y, 0, gauss_height-1), n) + (9) * f(clamp(size*x, 0, gauss_width-1), clamp(size*y+1, 0, gauss_height-1), n) + (3) * f(clamp(size*x, 0, gauss_width-1), clamp(size*y+2, 0, gauss_height-1), n) 
        //                 + (3) * f(clamp(size*x+1, 0, gauss_width-1), clamp(size*y-1, 0, gauss_height-1), n) + (9) * f(clamp(size*x+1, 0, gauss_width-1), clamp(size*y, 0, gauss_height-1), n) + (9) * f(clamp(size*x+1, 0, gauss_width-1), clamp(size*y+1, 0, gauss_height-1), n) + (3) * f(clamp(size*x+1, 0, gauss_width-1), clamp(size*y+2, 0, gauss_height-1), n) 
        //                 + (1) * f(clamp(size*x+2, 0, gauss_width-1), clamp(size*y-1, 0, gauss_height-1), n) + (3) * f(clamp(size*x+2, 0, gauss_width-1), clamp(size*y, 0, gauss_height-1), n) + (3) * f(clamp(size*x+2, 0, gauss_width-1), clamp(size*y+1, 0, gauss_height-1), n) + (1) * f(clamp(size*x+2, 0, gauss_width-1), clamp(size*y+2, 0, gauss_height-1), n);
        
      

        // USED
        // This produces negative indices. Need to find out what happens when you have negative indices???
        // Maybe do make run-cpu and trace stores?
        // Do make run-cpu and compare it to Kayvon's output 
        // down_pre_shift(x, y, n) = (1) * f(x_index_0, y_index_0, n) + (3) * f(x_index_0, y_index_1, n) + (3) * f(x_index_0, y_index_2, n) + (1) * f(x_index_0, y_index_3, n) 
        //                 + (3) * f(x_index_1, y_index_0, n) + (9) * f(x_index_1, y_index_1, n) + (9) * f(x_index_1, y_index_2, n) + (3) * f(x_index_1, y_index_3, n) 
        //                 + (3) * f(x_index_2, y_index_0, n) + (9) * f(x_index_2, y_index_1, n) + (9) * f(x_index_2, y_index_2, n) + (3) * f(x_index_2, y_index_3, n) 
        //                 + (1) * f(x_index_3, y_index_0, n) + (3) * f(x_index_3, y_index_1, n) + (3) * f(x_index_3, y_index_2, n) + (1) * f(x_index_3, y_index_3, n);


        // Expr is_edge_pixel = (x == 0 || x == 1 || x == 2 || y == 0 || y == 1 || y == 2 || x == (gauss_width-1) || x == (gauss_width-2) || x == (gauss_width-3) || y == (gauss_height-1) || y == (gauss_height-2) || y == (gauss_height=3));

        // down_pre_shift(x, y, n) = select((is_edge_pixel), f(x, y, n), (1) * f(x_index_0, y_index_0, n) + (3) * f(x_index_0, y_index_1, n) + (3) * f(x_index_0, y_index_2, n) + (1) * f(x_index_0, y_index_3, n) 
        //                 + (3) * f(x_index_1, y_index_0, n) + (9) * f(x_index_1, y_index_1, n) + (9) * f(x_index_1, y_index_2, n) + (3) * f(x_index_1, y_index_3, n) 
        //                 + (3) * f(x_index_2, y_index_0, n) + (9) * f(x_index_2, y_index_1, n) + (9) * f(x_index_2, y_index_2, n) + (3) * f(x_index_2, y_index_3, n) 
        //                 + (1) * f(x_index_3, y_index_0, n) + (3) * f(x_index_3, y_index_1, n) + (3) * f(x_index_3, y_index_2, n) + (1) * f(x_index_3, y_index_3, n));
      
      
      // // down_pre_shift(x, y, n) = (1) * f(size*x-1, size*y-1, n) + (3) * f(size*x-1, size*y, n) + (3) * f(size*x-1, size*y+1, n) + (1) * f(size*x-1, size*y+2, n) 
      // //                   + (3) * f(size*x, size*y-1, n) + (9) * f(size*x, size*y, n) + (9) * f(size*x, size*y+1, n) + (3) * f(size*x, size*y+2, n) 
      // //                   + (3) * f(size*x+1, size*y-1, n) + (9) * f(size*x+1, size*y, n) + (9) * f(size*x+1, size*y+1, n) + (3) * f(size*x+1, size*y+2, n) 
      // //                   + (1) * f(size*x+2, size*y-1, n) + (3) * f(size*x+2, size*y, n) + (3) * f(size*x+2, size*y+1, n) + (1) * f(size*x+2, size*y+2, n);

      //  /* Func: down
      //   * dtype: u16
      //   * True range: [0, 1023] 
      //   * Consumer(s): returned by downsample_u16_hdr
      //   */
        //down(x, y, n) = down_pre_shift(x, y, n) >> 6;

        down(x,y,n) = f(x*2, y*2, n);

        return down;
    }
  };
}
HALIDE_REGISTER_GENERATOR(HDRPlus, hdr_plus)