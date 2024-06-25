
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

    // Output a single 8-bit RGB image 
    Output<Buffer<uint8_t>> output{"output", 3};

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

        // initialGaussWidth[0] = 625;
        // initialGaussWidth[1] = 312;
        // initialGaussWidth[2] = 156;
        // initialGaussWidth[3] = 78;
        // initialGaussWidth[4] = 39;


        initialGaussWidth[0] = 256;
        initialGaussWidth[1] = 128;
        initialGaussWidth[2] = 64;
        initialGaussWidth[3] = 32;
        initialGaussWidth[4] = 16;

        // initialGaussHeight[0] = 560;
        // initialGaussHeight[1] = 280;
        // initialGaussHeight[2] = 140;
        // initialGaussHeight[3] = 70;
        // initialGaussHeight[4] = 35;


        initialGaussHeight[0] = 256;
        initialGaussHeight[1] = 128;
        initialGaussHeight[2] = 64;
        initialGaussHeight[3] = 32;
        initialGaussHeight[4] = 16;


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


        // gPyramid[4].bound(x, 0, 16);
        // gPyramid[4].bound(y, 0, 16);
        // gPyramid[4].bound(n, 0, 3);

        // gPyramid[3].bound(x, 0, 32);
        // gPyramid[3].bound(y, 0, 32);
        // gPyramid[3].bound(n, 0, 3);
    
        // Func provisional_output;
        // provisional_output(x, y, n) = gPyramid[4](x, y, n);
        // output(x, y, c) = u8(provisional_output(x, y, c));
        // output.bound(c, 0, 3);
        // output.bound(x, 0, 16);
        // output.bound(y, 0, 16);

        // STEP 3: Align pyramids and upsample the alignment back to the bottom layer 
        Func initialAlign;
        
        initialAlign(tx, ty, xy, n) = 0;
        // initialAlign.bound(tx, 0, 16);
        // initialAlign.bound(tx, 0, 16);
        // initialAlign.bound(xy, 0, 1);
        // initialAlign.bound(n, 0, 3);

        vector<Func> alignPyramid(J);
        Expr min_align[J];
        Expr max_align[J];
        Expr gauss_width[J];
        Expr gauss_height[J];
        min_align[J-1] = i16(Expr(-4));
        max_align[J-1] = i16(Expr(4));

        // gauss_width[4] = 39;
        // gauss_width[3] = 78;
        // gauss_width[2] = 156;
        // gauss_width[1] = 312;
        // gauss_width[0] = 625;

        gauss_width[4] = 16;
        gauss_width[3] = 32;
        gauss_width[2] = 64;
        gauss_width[1] = 128;
        gauss_width[0] = 256;

        // gauss_height[4] = 35;
        // gauss_height[3] = 70;
        // gauss_height[2] = 140;
        // gauss_height[1] = 280;
        // gauss_height[0] = 560;

        gauss_height[4] = 16;
        gauss_height[3] = 32;
        gauss_height[2] = 64;
        gauss_height[1] = 128;
        gauss_height[0] = 256;


        Expr upsample_flow_gauss_widths[J];
        Expr upsample_flow_gauss_heights[J];

        upsample_flow_gauss_widths[4] = 1; 
        upsample_flow_gauss_widths[3] = 1; 
        upsample_flow_gauss_widths[2] = 2; 
        upsample_flow_gauss_widths[1] = 4; 
        upsample_flow_gauss_widths[0] = 8; 
      
        upsample_flow_gauss_heights[4] = 1;
        upsample_flow_gauss_heights[3] = 1;
        upsample_flow_gauss_heights[2] = 2;
        upsample_flow_gauss_heights[1] = 4;
        upsample_flow_gauss_heights[0] = 8;

   
       /* ALIGN PYRAMID LEVEL 4*/
        //Var tx, ty, xy, n;
        Var x_s_lvl_4, y_s_lvl_4;

        RDom r_tile_lvl_4(0, T_SIZE, 0, T_SIZE);
        RDom r_search_lvl_4(-4, 9, -4, 9);
        

       /* Func: coarse_offset_lvl_4
        * dtype: i16
        * True range: [0]
        * Consumer(s): alignPyramid[4]
        */
        Func coarse_offset_lvl_4;
        coarse_offset_lvl_4(tx, ty, xy, n) = i16(2 * upsample_u16_size_2_for_alignment(initialAlign, upsample_flow_gauss_widths[4], upsample_flow_gauss_heights[4])(tx, ty, xy, n));

        Expr x_ref_lvl_4 = clamp(tx * T_SIZE + r_tile_lvl_4.x, 0, gauss_width[4]-1);
        Expr y_ref_lvl_4 = clamp(ty * T_SIZE + r_tile_lvl_4.y, 0, gauss_height[4]-1);

        Expr x_cmp_lvl_4 = clamp(tx * T_SIZE + r_tile_lvl_4.x + coarse_offset_lvl_4(tx, ty, 0, n) + x_s_lvl_4, 0, gauss_width[4]-1);
        Expr y_cmp_lvl_4 = clamp(ty * T_SIZE + r_tile_lvl_4.y + coarse_offset_lvl_4(tx, ty, 1, n) + y_s_lvl_4, 0, gauss_height[4]-1);

        Expr dist_lvl_4 = abs(i16(gPyramid[4](x_ref_lvl_4, y_ref_lvl_4, 0)) - i16(gPyramid[4](x_cmp_lvl_4, y_cmp_lvl_4, n))); 

       /* Func: scores_lvl_4
        * dtype: u32
        * True range: [0, 261888] (worst case)
        * Consumer(s): alignPyramid[4]
        */
        Func scores_lvl_4;
        scores_lvl_4(tx, ty, x_s_lvl_4, y_s_lvl_4, n) = sum(u32(dist_lvl_4));

        Func min_val_lvl_4, min_x_lvl_4, min_y_lvl_4;
        min_val_lvl_4(tx, ty, n) = cast<uint>(std::numeric_limits<int>::max());
        //min_val_lvl_4(tx, ty, n) = cast<uint16_t>(std::numeric_limits<int>::max());
        min_x_lvl_4(tx, ty, n) = 0;
        min_y_lvl_4(tx, ty, n) = 0;

        min_x_lvl_4.bound(tx, 0, 1);
        min_x_lvl_4.bound(ty, 0, 1);
        min_x_lvl_4.bound(n, 0, 3);

        min_y_lvl_4.bound(tx, 0, 1);
        min_y_lvl_4.bound(ty, 0, 1);
        min_y_lvl_4.bound(n, 0, 3);

        min_val_lvl_4.bound(tx, 0, 1);
        min_val_lvl_4.bound(ty, 0, 1);
        min_val_lvl_4.bound(n, 0, 3);

        // Update the minimum function and coordinates
        Expr condition = scores_lvl_4(tx, ty, r_search_lvl_4.x, r_search_lvl_4.y, n) < min_val_lvl_4(tx, ty, n);
        Expr new_min_lvl_4 = select(condition, scores_lvl_4(tx, ty, r_search_lvl_4.x, r_search_lvl_4.y, n), min_val_lvl_4(tx, ty, n));
        Expr new_min_x_lvl_4 = select(condition, r_search_lvl_4.x, min_x_lvl_4(tx, ty, n));
        Expr new_min_y_lvl_4 = select(condition, r_search_lvl_4.y, min_y_lvl_4(tx, ty, n));

        min_val_lvl_4(tx, ty, n) = new_min_lvl_4;
        min_x_lvl_4(tx, ty, n) = new_min_x_lvl_4;
        min_y_lvl_4(tx, ty, n) = new_min_y_lvl_4;


       /* Func: alignPyramid[4]
        * dtype: i16
        * True range: [-4, 4] (worst case)
        * Consumer(s): coarse_offset_lvl_3
        */
        alignPyramid[4](tx, ty, xy, n) = select(n == 0, i16(0),
                    xy == 0, i16(min_x_lvl_4(tx, ty, n)) + coarse_offset_lvl_4(tx, ty, 0, n),
                    i16(min_y_lvl_4(tx, ty, n)) + coarse_offset_lvl_4(tx, ty, 1, n));

        alignPyramid[4].bound(tx, 0, 1);
        alignPyramid[4].bound(ty, 0, 1);
        alignPyramid[4].bound(xy, 0, 1);
        alignPyramid[4].bound(n, 0, 3);

        Func provisional_output;
        provisional_output(x, y, n) = alignPyramid[4](x, y, 0, n);
        output(x, y, c) = u8(provisional_output(x, y, c));
        output.bound(c, 0, 3);
        output.bound(x, 0, 1);
        output.bound(y, 0, 1);



        // Schedule
        //Var xo("xo"), yo("yo"), xi("xi"), yi("yi"), outer("outer");

      if (get_target().has_feature(Target::CoreIR)) {

      } else if (get_target().has_feature(Target::Clockwork)) {
    
        const int output_x_size = 1;
        const int output_y_size = 1;

        output.bound(x, 0, output_x_size);
        output.bound(y, 0, output_y_size);

        provisional_output.in().compute_root();

        provisional_output.in().tile(x, y, xo, yo, xi, yi, output_x_size, output_y_size)
          .reorder(n, xi, yi, xo, yo)
          .hw_accelerate(xi, xo);

        provisional_output.tile(x, y, xo, yo, xi, yi, output_x_size, output_y_size)
          .reorder(n, xi, yi, xo, yo);
        provisional_output.compute_at(provisional_output.in(), xo);
        provisional_output.store_in(MemoryType::GLB);

        alignPyramid[4].compute_at(provisional_output, xo);

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

        hw_input.bound(x, 0, 512);
        hw_input.bound(y, 0, 512);
        hw_input.bound(n, 0, 3);
        hw_input.compute_root()
          .accelerator_input();

      } else {

        // // ALIGN SCHEDULE 
        // Var xo("xo"), yo("yo"), outer("outer");
        // align_output.reorder(xy, n, tx, ty).tile(tx, ty, xo, yo, xi, yi, 4, 4).fuse(xo, yo, outer).parallel(outer);
        // //align_output.reorder(xy, n, tx, ty).parallel(outer);
        // align_output.compute_root();
        // //deinterleaved.compute_root();
        // //hw_input_float.compute_root();
        // //deinterleaved.compute_root();
        // gray.compute_root().parallel(y, 32).vectorize(x, 8);
        
        // for (int j = 1; j < J; j++) {
        //     gPyramid[j]
        //         .compute_root().parallel(y, 8).vectorize(x, 8);


        //     // USE THIS FOR PRINTING COARSE OFFSET
        //     // if (j <= 0)
        //     //    alignPyramid[j]
        //     //   .store_at(align_output, outer).compute_at(align_output, yi);
        //     // else
        //     //   //alignPyramid[j]
        //     //   //.store_at(align_output, outer).compute_at(align_output, yi);
        //     //   alignPyramid[j].compute_at(coarse_offset_lvl_0, n);
            

        //     // USE THIS FOR PRINTING SCORES
        //     alignPyramid[j].compute_root();
        // }
        // //coarse_offset_lvl_4.compute_root();
        // //coarse_offset_lvl_3.compute_root();
        // //coarse_offset_lvl_2.compute_root();
        // //coarse_offset_lvl_1.compute_root();
        // //coarse_offset_lvl_0.compute_root();

        // scores_lvl_0.compute_root();

        // // alignPyramid[0-].compute_root()
        // alignPyramid[0].compute_at(align_output, yi);
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
        Expr x_index_0 = clamp(size*x-1, 0, gauss_width-1);
        Expr x_index_1 = clamp(size*x, 0, gauss_width-1);
        Expr x_index_2 = clamp(size*x+1, 0, gauss_width-1);
        Expr x_index_3 = clamp(size*x+2, 0, gauss_width-1);


        Expr y_index_0 = clamp(size*y-1, 0, gauss_height-1);
        Expr y_index_1 = clamp(size*y, 0, gauss_height-1);
        Expr y_index_2 = clamp(size*y+1, 0, gauss_height-1);
        Expr y_index_3 = clamp(size*y+2, 0, gauss_height-1);


       /* Func: down_pre_shift
        * dtype: u16
        * True range: [0, 65472] (worst case)
        * Consumer(s): down
        */
        down_pre_shift(x, y, n) = (1) * f(clamp(size*x-1, 0, gauss_width-1), clamp(size*y-1, 0, gauss_height-1), n) + (3) * f(clamp(size*x-1, 0, gauss_width-1), clamp(size*y, 0, gauss_height-1), n) + (3) * f(clamp(size*x-1, 0, gauss_width-1), clamp(size*y+1, 0, gauss_height-1), n) + (1) * f(clamp(size*x-1, 0, gauss_width-1), clamp(size*y+2, 0, gauss_height-1), n) 
                        + (3) * f(clamp(size*x, 0, gauss_width-1), clamp(size*y-1, 0, gauss_height-1), n) + (9) * f(clamp(size*x, 0, gauss_width-1), clamp(size*y, 0, gauss_height-1), n) + (9) * f(clamp(size*x, 0, gauss_width-1), clamp(size*y+1, 0, gauss_height-1), n) + (3) * f(clamp(size*x, 0, gauss_width-1), clamp(size*y+2, 0, gauss_height-1), n) 
                        + (3) * f(clamp(size*x+1, 0, gauss_width-1), clamp(size*y-1, 0, gauss_height-1), n) + (9) * f(clamp(size*x+1, 0, gauss_width-1), clamp(size*y, 0, gauss_height-1), n) + (9) * f(clamp(size*x+1, 0, gauss_width-1), clamp(size*y+1, 0, gauss_height-1), n) + (3) * f(clamp(size*x+1, 0, gauss_width-1), clamp(size*y+2, 0, gauss_height-1), n) 
                        + (1) * f(clamp(size*x+2, 0, gauss_width-1), clamp(size*y-1, 0, gauss_height-1), n) + (3) * f(clamp(size*x+2, 0, gauss_width-1), clamp(size*y, 0, gauss_height-1), n) + (3) * f(clamp(size*x+2, 0, gauss_width-1), clamp(size*y+1, 0, gauss_height-1), n) + (1) * f(clamp(size*x+2, 0, gauss_width-1), clamp(size*y+2, 0, gauss_height-1), n);
        

       /* Func: down
        * dtype: u16
        * True range: [0, 1023] 
        * Consumer(s): returned by downsample_u16_hdr
        */
        down(x, y, n) = down_pre_shift(x, y, n) >> 6;

        return down;
    }
  };
}
HALIDE_REGISTER_GENERATOR(HDRPlus, hdr_plus)