
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

class HDRPlus_kernel_1 : public Halide::Generator<HDRPlus_kernel_1> {
public:
    // FOR ALIGN
    GeneratorParam<int>     pyramid_levels{"pyramid_levels", 5};
    GeneratorParam<int>     total_downsample_factor{"total_downsample_factor", 16}; // 2^ pyramid_levels-1

    // Operate on raw bayer image: so 2 channels, plus 1 channel b/c receiving multiple images
    //Input<Buffer<uint16_t>>  input{"input", 3};
    Input<Buffer<int16_t>>  input{"input", 4};

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
        hw_input(x, y, xy, n) = clamped_input(x, y, xy, n);

        Func hw_input_copy;
        hw_input_copy(x, y, xy, n) = hw_input(x, y, xy, n) + i16(0); 
  
        const int input_x_size = 256;
        const int input_y_size = 256;

        const int output_x_size = 16;
        const int output_y_size = 16;

        Expr gauss_width = 512;
        Expr gauss_height = 512;


        // Expr gauss_width = 39;
        // Expr gauss_height = 35;

        Func gPyramid4_LUT;
        gPyramid4_LUT(x, y, n) = i16(x * 2 + y * 3 + n); 
        gPyramid4_LUT.bound(x, 0, gauss_width);
        gPyramid4_LUT.bound(y, 0, gauss_height);
        gPyramid4_LUT.bound(n, 0, 3);


        /* ALIGN PYRAMID LEVEL 4*/
        //Var tx, ty, xy, n;
        Var x_s_lvl_4, y_s_lvl_4;

        RDom r_tile_lvl_4(0, T_SIZE, 0, T_SIZE);
        RDom r_search_lvl_4(-4, 9, -4, 9);


        Expr upsample_flow_gauss_width = input_x_size;
        Expr upsample_flow_gauss_height = input_y_size; 

      
        

       /* Func: coarse_offset_lvl_4
        * dtype: i16
        * True range: [0]
        * Consumer(s): alignPyramid[4]
        */
        Func coarse_offset_lvl_4;
        //coarse_offset_lvl_4(tx, ty, xy, n) = i16(2 * upsample_u16_size_2_for_alignment(hw_input_copy, upsample_flow_gauss_width, upsample_flow_gauss_height)(tx, ty, xy, n));
        //coarse_offset_lvl_4(tx, ty, xy, n) = i16(0);

        // Expr x_ref_lvl_4 = clamp(tx * T_SIZE + r_tile_lvl_4.x, 0, gauss_width-1);
        // Expr y_ref_lvl_4 = clamp(ty * T_SIZE + r_tile_lvl_4.y, 0, gauss_height-1);

        // Expr x_ref_lvl_4 = tx * T_SIZE;
        // Expr y_ref_lvl_4 = ty * T_SIZE;


        // Expr x_ref_lvl_4 = (tx * T_SIZE) + r_tile_lvl_4.x;
        // Expr y_ref_lvl_4 = (ty * T_SIZE) + r_tile_lvl_4.y;

        Expr x_ref_lvl_4 = tx*T_SIZE + r_tile_lvl_4.x;
        Expr y_ref_lvl_4 = ty*T_SIZE + r_tile_lvl_4.y;

        // Expr x_cmp_lvl_4 = clamp(tx * T_SIZE + r_tile_lvl_4.x + coarse_offset_lvl_4(tx, ty, 0, n) + x_s_lvl_4, 0, gauss_width-1);
        // Expr y_cmp_lvl_4 = clamp(ty * T_SIZE + r_tile_lvl_4.y + coarse_offset_lvl_4(tx, ty, 1, n) + y_s_lvl_4, 0, gauss_height-1);

        // Expr x_cmp_lvl_4 = tx * T_SIZE + coarse_offset_lvl_4(tx, ty, 0, n);
        // Expr y_cmp_lvl_4 = ty * T_SIZE + coarse_offset_lvl_4(tx, ty, 1, n);


        // Expr x_cmp_lvl_4 = clamp(tx * T_SIZE + r_tile_lvl_4.x + coarse_offset_lvl_4(tx, ty, 0, n), 0, gauss_width-1);
        // Expr y_cmp_lvl_4 = clamp(ty * T_SIZE + r_tile_lvl_4.y + coarse_offset_lvl_4(tx, ty, 1, n), 0, gauss_height-1);

        // Expr x_cmp_lvl_4 = clamp(tx * T_SIZE + r_tile_lvl_4.x + x_s_lvl_4, 0, gauss_width[4]-1);
        // Expr y_cmp_lvl_4 = clamp(ty * T_SIZE + r_tile_lvl_4.y + y_s_lvl_4, 0, gauss_height[4]-1);

        //Expr dist_lvl_4 = abs(i16(gPyramid4_LUT(x_ref_lvl_4, y_ref_lvl_4, 0)) - i16(gPyramid4_LUT(x_cmp_lvl_4, y_cmp_lvl_4, n))); 
        //Expr dist_lvl_4 = abs(i16(gPyramid4_LUT(x_ref_lvl_4, y_ref_lvl_4, 0)) - i16(gPyramid4_LUT(coarse_offset_lvl_4(tx, ty, 0, n), coarse_offset_lvl_4(tx, ty, 1, n), n))); 

        //Expr dist_lvl_4 = abs(i16(gPyramid4_LUT(x_ref_lvl_4, y_ref_lvl_4, n))); 
        Expr dist_lvl_4 = abs(i16(hw_input(x_ref_lvl_4, y_ref_lvl_4, 0, n))); 


       /* Func: scores_lvl_4
        * dtype: u32
        * True range: [0, 261888] (worst case)
        * Consumer(s): alignPyramid[4]
        */
        Func scores_lvl_4;
        //scores_lvl_4(tx, ty, x_s_lvl_4, y_s_lvl_4, n) = sum(u32(dist_lvl_4));
        //scores_lvl_4(tx, ty, n) = sum(u16(dist_lvl_4));
        //scores_lvl_4(tx, ty, n) = u16(dist_lvl_4);
        scores_lvl_4(tx, ty, n) = u16(0);
        scores_lvl_4(tx, ty, n) += u16(dist_lvl_4);

        //scores_lvl_4(tx, ty, n) = sum(u16(dist_lvl_4));


        // Func min_val_lvl_4, min_x_lvl_4, min_y_lvl_4;
        // min_val_lvl_4(tx, ty, n) = cast<uint>(std::numeric_limits<int>::max());
        // //min_val_lvl_4(tx, ty, n) = cast<uint16_t>(std::numeric_limits<int>::max());
        // min_x_lvl_4(tx, ty, n) = 0;
        // min_y_lvl_4(tx, ty, n) = 0;

        // // Update the minimum function and coordinates
        // Expr condition = scores_lvl_4(tx, ty, r_search_lvl_4.x, r_search_lvl_4.y, n) < min_val_lvl_4(tx, ty, n);
        // Expr new_min_lvl_4 = select(condition, scores_lvl_4(tx, ty, r_search_lvl_4.x, r_search_lvl_4.y, n), min_val_lvl_4(tx, ty, n));
        // Expr new_min_x_lvl_4 = select(condition, r_search_lvl_4.x, min_x_lvl_4(tx, ty, n));
        // Expr new_min_y_lvl_4 = select(condition, r_search_lvl_4.y, min_y_lvl_4(tx, ty, n));

        // min_val_lvl_4(tx, ty, n) = new_min_lvl_4;
        // min_x_lvl_4(tx, ty, n) = new_min_x_lvl_4;
        // min_y_lvl_4(tx, ty, n) = new_min_y_lvl_4;

        Func alignPyramid_4_x, alignPyramid_4_y;
        // alignPyramid_4_x(tx, ty, n) = i16(min_x_lvl_4(tx, ty, n)) + coarse_offset_lvl_4(tx, ty, 0, n);
        // alignPyramid_4_y(tx, ty, n) = i16(min_y_lvl_4(tx, ty, n)) + coarse_offset_lvl_4(tx, ty, 1, n);

        // alignPyramid_4_x(tx, ty, n) = i16(scores_lvl_4(tx, ty, n)) + coarse_offset_lvl_4(tx, ty, 0, n);
        // alignPyramid_4_y(tx, ty, n) = i16(scores_lvl_4(tx, ty, n)) + coarse_offset_lvl_4(tx, ty, 1, n);

        alignPyramid_4_x(tx, ty, n) = i16(scores_lvl_4(tx, ty, n));
        alignPyramid_4_y(tx, ty, n) = i16(scores_lvl_4(tx, ty, n));




       /* Func: alignPyramid_4
        * dtype: i16
        * True range: [-4, 4] (worst case)
        * Consumer(s): coarse_offset_lvl_3
        */
        Func alignPyramid_4;
        alignPyramid_4(tx, ty, xy, n) = select(xy == 0, alignPyramid_4_x(tx, ty, n), alignPyramid_4_y(tx, ty, n));


        // CHANGE THESE BOUNDS 
        Func provisional_output;
        provisional_output(x, y) = alignPyramid_4(x, y, 0, 0);
        output(x, y) = u8(provisional_output(x, y));
        //output.bound(n, 0, 3);
        output.bound(x, 0, output_x_size);
        output.bound(y, 0, output_y_size);

      // Schedule
      if (get_target().has_feature(Target::CoreIR)) {

      } else if (get_target().has_feature(Target::Clockwork)) {

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

        alignPyramid_4.compute_at(provisional_output, xo);
        alignPyramid_4_y.compute_at(provisional_output, xo);
        alignPyramid_4_x.compute_at(provisional_output, xo);
        scores_lvl_4.compute_at(provisional_output, xo);
        //coarse_offset_lvl_4.compute_at(provisional_output, xo);
   

        hw_input_copy.compute_at(provisional_output, xo);
  
        hw_input.in().in().compute_at(provisional_output, xo); // represents the mem tile

        hw_input.in().compute_at(provisional_output.in(), xo); // represents the glb level
        hw_input.in().store_in(MemoryType::GLB);

        hw_input.bound(x, 0, input_x_size);
        hw_input.bound(y, 0, input_y_size);
        hw_input.bound(n, 0, 3);
        hw_input.compute_root()
          .accelerator_input();


      // SCHEDULE TO CPU
      } else {

        // gPyramid[0].compute_root();
        // gPyramid[1].compute_root();
        // gPyramid[2].compute_root();

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
        down_pre_shift(x, y, n) = (1) * f(x_index_0, y_index_0, n) + (3) * f(x_index_0, y_index_1, n) + (3) * f(x_index_0, y_index_2, n) + (1) * f(x_index_0, y_index_3, n) 
                        + (3) * f(x_index_1, y_index_0, n) + (9) * f(x_index_1, y_index_1, n) + (9) * f(x_index_1, y_index_2, n) + (3) * f(x_index_1, y_index_3, n) 
                        + (3) * f(x_index_2, y_index_0, n) + (9) * f(x_index_2, y_index_1, n) + (9) * f(x_index_2, y_index_2, n) + (3) * f(x_index_2, y_index_3, n) 
                        + (1) * f(x_index_3, y_index_0, n) + (3) * f(x_index_3, y_index_1, n) + (3) * f(x_index_3, y_index_2, n) + (1) * f(x_index_3, y_index_3, n);


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
        down(x, y, n) = down_pre_shift(x, y, n) >> 6;

        //down(x,y,n) = f(x*2, y*2, n);

        return down;
    }
  };
}
HALIDE_REGISTER_GENERATOR(HDRPlus_kernel_1, hdr_plus_kernel_1)