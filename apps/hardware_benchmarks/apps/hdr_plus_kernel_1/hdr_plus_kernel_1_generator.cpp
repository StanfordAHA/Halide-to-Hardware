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
    //GeneratorParam<int>     pyramid_levels{"pyramid_levels", 4, 1, maxJ};
    GeneratorParam<int>     pyramid_levels{"pyramid_levels", 5};
    GeneratorParam<int>     total_downsample_factor{"total_downsample_factor", 16}; // 2^ pyramid_levels-1
  
    // Receive alignPyramid as input, so 4 channels: tx, ty, xy, n
    Input<Buffer<int16_t>>  input{"input", 4};

    // Output 
    Output<Buffer<uint8_t>> output{"output", 3};

    void generate() {
        // Algorithm
    
        /* 
         * ALIGN STEP
         */
        const int J = pyramid_levels;

       /* Func: clamped_input
        * dtype: i16
        * True range: [-4, 4] (worst case)
        * Consumer(s): hw_input
        */
        Func clamped_input;
        clamped_input = Halide::BoundaryConditions::repeat_edge(input);

       /* Func: hw_input
        * dtype: i16
        * True range: [-4, 4] (worst case)
        * Consumer(s): coarse_offset_lvl_3
        */
        Func hw_input;
        hw_input(x, y, xy, n) = clamped_input(x, y, xy, n);

        Func hw_input_copy;
        hw_input_copy(x, y, xy, n) = hw_input(x, y, xy, n) + i16(0); 

        Func gPyramid4_LUT;
        gPyramid4_LUT(x, y, n) = u16(x * 2 + y * 3 + n); 
        gPyramid4_LUT.bound(x, 0, 39);
        gPyramid4_LUT.bound(y, 0, 35);
        gPyramid4_LUT.bound(n, 0, 3);
        //gPyramid4_LUT.trace_stores();

        Expr gauss_width_4, gauss_height_4;
        gauss_width_4 = 39;
        gauss_height_4 = 35;

        Expr upsample_flow_gauss_width_4, upsample_flow_gauss_height_4;
        upsample_flow_gauss_width_4 = 2;
        upsample_flow_gauss_height_4 = 2;

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
        coarse_offset_lvl_4(tx, ty, xy, n) = i16(2 * upsample_u16_size_2_for_alignment(hw_input_copy, upsample_flow_gauss_width_4, upsample_flow_gauss_height_4)(tx, tx, xy, n));

        Expr x_ref_lvl_4 = clamp(tx * T_SIZE + r_tile_lvl_4.x, 0, gauss_width_4-1);
        Expr y_ref_lvl_4 = clamp(ty * T_SIZE + r_tile_lvl_4.y, 0, gauss_height_4-1);

        Expr x_cmp_lvl_4 = clamp(tx * T_SIZE + r_tile_lvl_4.x + coarse_offset_lvl_4(tx, ty, 0, n) + x_s_lvl_4, 0, gauss_width_4-1);
        Expr y_cmp_lvl_4 = clamp(ty * T_SIZE + r_tile_lvl_4.y + coarse_offset_lvl_4(tx, ty, 1, n) + y_s_lvl_4, 0, gauss_height_4-1);

        Expr dist_lvl_4 = abs(i16(gPyramid4_LUT(x_ref_lvl_4, y_ref_lvl_4, 0)) - i16(gPyramid4_LUT(x_cmp_lvl_4, y_cmp_lvl_4, n))); 

       /* Func: scores_lvl_4
        * dtype: u32
        * True range: [0, 261888] (worst case)
        * Consumer(s): alignPyramid[4]
        */
        Func scores_lvl_4;
        scores_lvl_4(tx, ty, x_s_lvl_4, y_s_lvl_4, n) = sum(u32(dist_lvl_4));

        Func min_val_lvl_4, min_x_lvl_4, min_y_lvl_4;
        min_val_lvl_4(tx, ty, n) = cast<uint>(std::numeric_limits<int>::max());
        //min_val_lvl_4(tx, ty, n) = u32(300000);
        min_x_lvl_4(tx, ty, n) = 0;
        min_y_lvl_4(tx, ty, n) = 0;


        Expr curr_score = scores_lvl_4(tx, ty, r_search_lvl_4.x, r_search_lvl_4.y, n);

        // Update the minimum function and coordinates
        Expr new_min_lvl_4 = select(curr_score < min_val_lvl_4(tx, ty, n), curr_score, min_val_lvl_4(tx, ty, n));
        Expr new_min_x_lvl_4 = select(curr_score < min_val_lvl_4(tx, ty, n), r_search_lvl_4.x, min_x_lvl_4(tx, ty, n));
        Expr new_min_y_lvl_4 = select(curr_score < min_val_lvl_4(tx, ty, n), r_search_lvl_4.y, min_y_lvl_4(tx, ty, n));

        min_val_lvl_4(tx, ty, n) = new_min_lvl_4;
        min_x_lvl_4(tx, ty, n) = new_min_x_lvl_4;
        min_y_lvl_4(tx, ty, n) = new_min_y_lvl_4;


       /* Func: alignPyramid[4]
        * dtype: i16
        * True range: [-4, 4] (worst case)
        * Consumer(s): coarse_offset_lvl_3
        */
        Func x_align_output_4, y_align_output_4;
        x_align_output_4(tx, ty, xy, n) = i16(min_x_lvl_4(tx, ty, n)) + coarse_offset_lvl_4(tx, ty, 0, n);
        y_align_output_4(tx, ty, xy, n) = i16(min_y_lvl_4(tx, ty, n)) + coarse_offset_lvl_4(tx, ty, 1, n);

        Func alignPyramid_4;
        alignPyramid_4(tx, ty, xy, n) = select(n == 0, i16(0),
                    xy == 0, x_align_output_4(tx, ty, xy, n),
                             y_align_output_4(tx, ty, xy, n));


        Func provisional_output;
        provisional_output(x, y, n) = alignPyramid_4(x, y, 0, n);
        output(x, y, c) = u8(provisional_output(x, y, c));
        output.bound(c, 0, 3);
        output.bound(x, 0, 3);
        output.bound(y, 0, 3);

      // Schedule
      if (get_target().has_feature(Target::CoreIR)) {

      } else if (get_target().has_feature(Target::Clockwork)) {
    
        const int output_x_size = 3;
        const int output_y_size = 3;

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

        alignPyramid_4.compute_at(provisional_output, xo);
        scores_lvl_4.compute_at(provisional_output, xo);    
        
        hw_input_copy.compute_at(provisional_output, xo);

        hw_input.in().in().compute_at(provisional_output, xo); // represents the mem tile
        hw_input.in().compute_at(provisional_output.in(), xo); // represents the glb level
        hw_input.in().store_in(MemoryType::GLB);

        hw_input.bound(x, 0, 2);
        hw_input.bound(y, 0, 2);
        hw_input.bound(xy, 0, 2);
        hw_input.bound(n, 0, 3);
        hw_input.compute_root()
          .accelerator_input();
    
      // SCHEDULE TO CPU
      } else {
        gPyramid4_LUT.compute_root();

      }
    }
private:
    //Var x, y, tx, ty, xy, xi, yi, c, n;


    Func upsample_u16_size_2_for_alignment(Func f_in, Expr gauss_width, Expr gauss_height) {
      Var tx, ty, xy, n;
      using Halide::_;
      Func up, up_pre_shift, f_in_shift;


     /* Func: up_pre_shift
      * dtype: u16
      * True range: [0, 16368] (worst case)
      * Consumer(s): up
      */

      // Expr ty_mod_2 = select((ty & 1) == 0, 0, 1);
      // Expr tx_mod_2 = select((tx & 1) == 0, 0, 1);
      // //Expr tx_is_even = ((tx & 1) != 0);
      // // up_pre_shift(tx, ty, xy, n) = (9) * f_in(tx/2, ty/2, xy, n) 
      // // + (3) * f_in(tx/2, clamp((ty/2) - 1 + 2*(ty % 2), 0, gauss_height-1), xy, n)
      // // + (3) * f_in(clamp((tx/2) - 1 + 2*(tx % 2), 0, gauss_width-1), ty/2, xy, n) 
      // // + (1) * f_in(clamp((tx/2) - 1 + 2*(tx % 2), 0, gauss_width-1), clamp((ty/2) - 1 + 2*(ty % 2), 0, gauss_height-1), xy, n);

      // up_pre_shift(tx, ty, xy, n) = (9) * f_in(tx/2, ty/2, xy, n) 
      // + (3) * f_in(tx/2, clamp((ty/2) - 1 + 2*(ty_mod_2), 0, gauss_height-1), xy, n)
      // + (3) * f_in(clamp((tx/2) - 1 + 2*(tx_mod_2), 0, gauss_width-1), ty/2, xy, n) 
      // + (1) * f_in(clamp((tx/2) - 1 + 2*(tx_mod_2), 0, gauss_width-1), clamp((ty/2) - 1 + 2*(ty_mod_2), 0, gauss_height-1), xy, n);


      // // up_pre_shift(tx, ty, xy, n) = (9) * f_in(tx/2, ty/2, xy, n) 
      // // + (3) * f_in(tx/2, (ty/2) - 1 + 2*(ty % 2), xy, n)
      // // + (3) * f_in((tx/2) - 1 + 2*(tx % 2), ty/2, xy, n) 
      // // + (1) * f_in((tx/2) - 1 + 2*(tx % 2), (ty/2) - 1 + 2*(ty % 2), xy, n);

      
      //  /* Func: up
      //   * dtype: u16
      //   * True range: [0, 1023] 
      //   * Consumer(s): returned by upsample_u16_size_2_for_alignment
      //   */
      // up(tx, ty, xy, n) = up_pre_shift(tx, ty, xy, n) >> 4;
      // //up.bound(tx, 0, gauss_width);
      // // up.bound(ty, 0, gauss_height);
      // // up.bound(xy, 0, 2);
      // // up.bound(n, 0, 3);

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
HALIDE_REGISTER_GENERATOR(HDRPlus, hdr_plus_kernel_1)