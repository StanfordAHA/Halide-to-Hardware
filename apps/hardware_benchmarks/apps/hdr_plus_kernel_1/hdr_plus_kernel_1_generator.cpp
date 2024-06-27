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
  
    // Receive alignPyramid[4] as input, so 4 channels: tx, ty, xy, n
    //Input<Buffer<int16_t>>  input{"input", 4};
    Input<Buffer<int16_t>>  input{"input", 3};

    // Output a single 8-bit RGB image 
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
        // hw_input(x, y, xy, n) = clamped_input(x, y, xy, n);
        hw_input(x, y, n) = clamped_input(x, y, n);

        Func hw_input_copy;
        // hw_input_copy(x, y, xy, n) = hw_input(x, y, xy, n) + i16(0); 
        hw_input_copy(x, y, n) = hw_input(x, y, n) + i16(0); 



        // TODO: HOW DO I MAKE SURE THESE ARE OF THE RIGHT SIZE???
        Func gPyramid3_LUT;
        gPyramid3_LUT(x, y, n) = u16(x * 2 + y * 3 + n); 
        gPyramid3_LUT.bound(x, 0, 78);
        gPyramid3_LUT.bound(y, 0, 70);
        gPyramid3_LUT.bound(n, 0, 3);


        Func gPyramid2_LUT;
        gPyramid2_LUT(x, y, n) = u16(x * 1 + y * 3 + n); 
        gPyramid2_LUT.bound(x, 0, 156);
        gPyramid2_LUT.bound(y, 0, 140);
        gPyramid2_LUT.bound(n, 0, 3);


        Func gPyramid1_LUT;
        gPyramid1_LUT(x, y, n) = u16(x * 1 + y * 2 + n); 
        gPyramid1_LUT.bound(x, 0, 312);
        gPyramid1_LUT.bound(y, 0, 280);
        gPyramid1_LUT.bound(n, 0, 3);

        Func gPyramid0_LUT;
        gPyramid0_LUT(x, y, n) = u16(x * 2 + y * 1 + n); 
        gPyramid0_LUT.bound(x, 0, 625);
        gPyramid0_LUT.bound(y, 0, 560);
        gPyramid0_LUT.bound(n, 0, 3);


        vector<Func> alignPyramid_y(J);
        vector<Func> alignPyramid_x(J);
        Expr min_align[J];
        Expr max_align[J];
        Expr gauss_width[J];
        Expr gauss_height[J];
        min_align[J-1] = i16(Expr(-4));
        max_align[J-1] = i16(Expr(4));


        gauss_width[4] = 39;
        gauss_width[3] = 78;
        gauss_width[2] = 156;
        gauss_width[1] = 312;
        gauss_width[0] = 625;

        // gauss_width[4] = 16;
        // gauss_width[3] = 32;
        // gauss_width[2] = 64;
        // gauss_width[1] = 128;
        // gauss_width[0] = 256;



        gauss_height[4] = 35;
        gauss_height[3] = 70;
        gauss_height[2] = 140;
        gauss_height[1] = 280;
        gauss_height[0] = 560;


        // gauss_height[4] = 16;
        // gauss_height[3] = 32;
        // gauss_height[2] = 64;
        // gauss_height[1] = 128;
        // gauss_height[0] = 256;


        Expr upsample_flow_gauss_widths[J];
        Expr upsample_flow_gauss_heights[J];

        // upsample_flow_gauss_widths[4] = 1; 
        // upsample_flow_gauss_widths[3] = 1; 
        // upsample_flow_gauss_widths[2] = 2; 
        // upsample_flow_gauss_widths[1] = 4; 
        // upsample_flow_gauss_widths[0] = 8; 
      
        // upsample_flow_gauss_heights[4] = 1;
        // upsample_flow_gauss_heights[3] = 1;
        // upsample_flow_gauss_heights[2] = 2;
        // upsample_flow_gauss_heights[1] = 4;
        // upsample_flow_gauss_heights[0] = 8;

        upsample_flow_gauss_widths[4] = 2; 
        upsample_flow_gauss_widths[3] = 3; 
        upsample_flow_gauss_widths[2] = 5; 
        upsample_flow_gauss_widths[1] = 10; 
        upsample_flow_gauss_widths[0] = 20; 
      
        upsample_flow_gauss_heights[4] = 2;
        upsample_flow_gauss_heights[3] = 3;
        upsample_flow_gauss_heights[2] = 5;
        upsample_flow_gauss_heights[1] = 9;
        upsample_flow_gauss_heights[0] = 18;



       /* ALIGN PYRAMID LEVEL 3*/
        Var x_s_lvl_3, y_s_lvl_3;
        
        RDom r_tile_lvl_3(0, T_SIZE, 0, T_SIZE);
        RDom r_search_lvl_3(-4, 9, -4, 9);
        
        
       /* Func: coarse_offset_lvl_3
        * dtype: i16
        * True range: [-8, 8]
        * Consumer(s): alignPyramid[3]
        */
        Func coarse_offset_lvl_3_y, coarse_offset_lvl_3_x;
        // coarse_offset_lvl_3_x(tx, ty, n) = i16(2 * combined_upsample_u16_size_2_for_alignment(hw_input_copy, upsample_flow_gauss_widths[3], upsample_flow_gauss_heights[3])(tx, ty, 0, n));
        // coarse_offset_lvl_3_y(tx, ty, n) = i16(2 * combined_upsample_u16_size_2_for_alignment(hw_input_copy, upsample_flow_gauss_widths[3], upsample_flow_gauss_heights[3])(tx, ty, 1, n));

        coarse_offset_lvl_3_x(tx, ty, n) = i16(2 * upsample_u16_size_2_for_alignment(hw_input_copy, upsample_flow_gauss_widths[3], upsample_flow_gauss_heights[3])(tx, ty, n));
        coarse_offset_lvl_3_y(tx, ty, n) = i16(2 * upsample_u16_size_2_for_alignment(hw_input_copy, upsample_flow_gauss_widths[3], upsample_flow_gauss_heights[3])(tx, ty, n));

        coarse_offset_lvl_3_x.bound(tx, 0, 5);
        coarse_offset_lvl_3_x.bound(ty, 0, 5);
        coarse_offset_lvl_3_x.bound(n, 0, 3);

        coarse_offset_lvl_3_y.bound(tx, 0, 5);
        coarse_offset_lvl_3_y.bound(ty, 0, 5);
        coarse_offset_lvl_3_y.bound(n, 0, 3);

        Expr x_ref_lvl_3 = clamp(tx * T_SIZE + r_tile_lvl_3.x, 0, gauss_width[3]-1);
        Expr y_ref_lvl_3 = clamp(ty * T_SIZE + r_tile_lvl_3.y, 0, gauss_height[3]-1);


        // Figure out if clamping is supported: I think clockwork figures this out b/c it pre-computes the address and schedule
        // Figure out how to pass data to another mem tile as the base address 
        Expr x_cmp_lvl_3 = clamp(tx * T_SIZE + r_tile_lvl_3.x + coarse_offset_lvl_3_x(tx, ty, n) + x_s_lvl_3, 0, gauss_width[3]-1);
        Expr y_cmp_lvl_3 = clamp(ty * T_SIZE + r_tile_lvl_3.y + coarse_offset_lvl_3_y(tx, ty, n) + y_s_lvl_3, 0, gauss_height[3]-1);

        // Expr x_cmp_lvl_3 = clamp(tx * T_SIZE + r_tile_lvl_3.x + x_s_lvl_3, 0, gauss_width[3]-1);
        // Expr y_cmp_lvl_3 = clamp(ty * T_SIZE + r_tile_lvl_3.y + y_s_lvl_3, 0, gauss_height[3]-1);

        //Expr dist_lvl_3 = abs(i16(gPyramid[3](x_ref_lvl_3, y_ref_lvl_3, 0)) - i16(gPyramid[3](x_cmp_lvl_3, y_cmp_lvl_3, n))); 
        Expr dist_lvl_3 = abs(i16(gPyramid3_LUT(x_ref_lvl_3, y_ref_lvl_3, 0)) - i16(gPyramid3_LUT(x_cmp_lvl_3, y_cmp_lvl_3, n))); 
        //Expr dist_lvl_3 = abs(i16(lut_values(x_ref_lvl_3, y_ref_lvl_3, 0)) - i16(lut_values(x_cmp_lvl_3, y_cmp_lvl_3, n))); 

       /* Func: scores_lvl_3
        * dtype: u32
        * True range: [0, 261888] (worst case)
        * Consumer(s): alignPyramid[3]
        */
        Func scores_lvl_3;
        scores_lvl_3(tx, ty, x_s_lvl_3, y_s_lvl_3, n) = sum(u32(dist_lvl_3));
        //scores_lvl_3(tx, tx, x_s_lvl_3, y_s_lvl_3, n) = sum(u16(dist_lvl_3));
        scores_lvl_3.bound(tx, 0, 5);
        scores_lvl_3.bound(ty, 0, 5);
        scores_lvl_3.bound(n, 0, 3);


        Func min_val_lvl_3, min_x_lvl_3, min_y_lvl_3;
        min_val_lvl_3(tx, ty, n) = cast<uint>(std::numeric_limits<int>::max());
        //min_val_lvl_3(tx, tx, n) = cast<uint16_t>(std::numeric_limits<int>::max());
        min_x_lvl_3(tx, ty, n) = 0;
        min_y_lvl_3(tx, ty, n) = 0;

        min_x_lvl_3.bound(tx, 0, 5);
        min_x_lvl_3.bound(ty, 0, 5);
        min_x_lvl_3.bound(n, 0, 3);

        min_y_lvl_3.bound(tx, 0, 5);
        min_y_lvl_3.bound(ty, 0, 5);
        min_y_lvl_3.bound(n, 0, 3);

        min_val_lvl_3.bound(tx, 0, 5);
        min_val_lvl_3.bound(ty, 0, 5);
        min_val_lvl_3.bound(n, 0, 3);


        // Update the minimum function and coordinates
        Expr new_min_lvl_3 = select(scores_lvl_3(tx, ty, r_search_lvl_3.x, r_search_lvl_3.y, n) < min_val_lvl_3(tx, ty, n), scores_lvl_3(tx, ty, r_search_lvl_3.x, r_search_lvl_3.y, n), min_val_lvl_3(tx, ty, n));
        Expr new_min_x_lvl_3 = select(scores_lvl_3(tx, ty, r_search_lvl_3.x, r_search_lvl_3.y, n) < min_val_lvl_3(tx, ty, n), r_search_lvl_3.x, min_x_lvl_3(tx, ty, n));
        Expr new_min_y_lvl_3 = select(scores_lvl_3(tx, ty, r_search_lvl_3.x, r_search_lvl_3.y, n) < min_val_lvl_3(tx, ty, n), r_search_lvl_3.y, min_y_lvl_3(tx, ty, n));

        min_val_lvl_3(tx, ty, n) = new_min_lvl_3;
        min_x_lvl_3(tx, ty, n) = new_min_x_lvl_3;
        min_y_lvl_3(tx, ty, n) = new_min_y_lvl_3;

        //Tuple min_coor_lvl_3 = argmin(scores_lvl_3(tx, tx, r_search_lvl_3.x, r_search_lvl_3.y, n));

       /* Func: alignPyramid[3]
        * dtype: i16
        * True range: [-4, 4] (worst case)
        * Consumer(s): coarse_offset_lvl_3
        */
        alignPyramid_y[3](tx, ty, n) = select(n == 0, i16(0), i16(min_y_lvl_3(tx, ty, n)) + coarse_offset_lvl_3_y(tx, ty, n));
        alignPyramid_x[3](tx, ty, n) = select(n == 0, i16(0), i16(min_x_lvl_3(tx, ty, n)) + coarse_offset_lvl_3_x(tx, ty, n));

        // alignPyramid[3](tx, ty, xy, n) = select(n == 0, i16(0),
        //             xy == 0, i16(min_x_lvl_3(tx, ty, n)) + coarse_offset_lvl_3_x(tx, ty, n),
        //             i16(min_y_lvl_3(tx, ty, n)) + coarse_offset_lvl_3_y(tx, ty, n));


        alignPyramid_y[3].bound(tx, 0, 5);
        alignPyramid_y[3].bound(ty, 0, 5);
        alignPyramid_y[3].bound(n, 0, 3);

        alignPyramid_x[3].bound(tx, 0, 5);
        alignPyramid_x[3].bound(ty, 0, 5);
        alignPyramid_x[3].bound(n, 0, 3);


        // alignPyramid[3].bound(tx, 0, 5);
        // alignPyramid[3].bound(ty, 0, 5);
        // // alignPyramid[3].bound(xy, 0, 1);
        // alignPyramid[3].bound(n, 0, 3);

        //alignPyramid[3].trace_stores();


        /* ALIGN PYRAMID LEVEL 2*/
        //Var tx, ty, xy, n;
        Var x_s_lvl_2, y_s_lvl_2;
      
        RDom r_tile_lvl_2(0, T_SIZE, 0, T_SIZE);
        RDom r_search_lvl_2(-4, 9, -4, 9);
        
        //coarse_offset_lvl_2(tx, ty, xy, n) = i32(ceil(2 * upsample_float_size_2_for_alignment(alignPyramid[3], upsample_flow_gauss_widths[2], upsample_flow_gauss_heights[2])(tx, ty, xy, n)));
        
       /* Func: coarse_offset_lvl_2
        * dtype: i16
        * True range: [-24, 24]
        * Consumer(s): alignPyramid[2]
        */
        Func coarse_offset_lvl_2_y, coarse_offset_lvl_2_x;
        coarse_offset_lvl_2_x(tx, ty, n) = i16(2 * upsample_u16_size_2_for_alignment(alignPyramid_x[3], upsample_flow_gauss_widths[2], upsample_flow_gauss_heights[2])(tx, ty, n));
        coarse_offset_lvl_2_y(tx, ty, n) = i16(2 * upsample_u16_size_2_for_alignment(alignPyramid_y[3], upsample_flow_gauss_widths[2], upsample_flow_gauss_heights[2])(tx, ty, n));

        coarse_offset_lvl_2_x.bound(tx, 0, 10);
        coarse_offset_lvl_2_x.bound(ty, 0, 9);
        coarse_offset_lvl_2_x.bound(n, 0, 3);

        coarse_offset_lvl_2_y.bound(tx, 0, 10);
        coarse_offset_lvl_2_y.bound(ty, 0, 9);
        coarse_offset_lvl_2_y.bound(n, 0, 3);

        Expr x_ref_lvl_2 = clamp(tx * T_SIZE + r_tile_lvl_2.x, 0, gauss_width[2]-1);
        Expr y_ref_lvl_2 = clamp(ty * T_SIZE + r_tile_lvl_2.y, 0, gauss_height[2]-1);

        Expr x_cmp_lvl_2 = clamp(tx * T_SIZE + r_tile_lvl_2.x + coarse_offset_lvl_2_x(tx, ty, n) + x_s_lvl_2, 0, gauss_width[2]-1);
        Expr y_cmp_lvl_2 = clamp(ty * T_SIZE + r_tile_lvl_2.y + coarse_offset_lvl_2_y(tx, ty, n) + y_s_lvl_2, 0, gauss_height[2]-1);

        // Expr x_cmp_lvl_2 = clamp(tx * T_SIZE + r_tile_lvl_2.x + x_s_lvl_2, 0, gauss_width[2]-1);
        // Expr y_cmp_lvl_2 = clamp(ty * T_SIZE + r_tile_lvl_2.y + y_s_lvl_2, 0, gauss_height[2]-1);   

        // Expr dist_lvl_2 = abs(i16(gPyramid[2](x_ref_lvl_2, y_ref_lvl_2, 0)) - i16(gPyramid[2](x_cmp_lvl_2, y_cmp_lvl_2, n))); 
        Expr dist_lvl_2 = abs(i16(gPyramid2_LUT(x_ref_lvl_2, y_ref_lvl_2, 0)) - i16(gPyramid2_LUT(x_cmp_lvl_2, y_cmp_lvl_2, n))); 


       /* Func: scores_lvl_2
        * dtype: u32
        * True range: [0, 261888] (worst case)
        * Consumer(s): alignPyramid[2]
        */
        Func scores_lvl_2;
        scores_lvl_2(tx, ty, x_s_lvl_2, y_s_lvl_2, n) = sum(u32(dist_lvl_2));
        //scores_lvl_2(tx, ty, x_s_lvl_2, y_s_lvl_2, n) = sum(u16(dist_lvl_2));
        scores_lvl_3.bound(tx, 0, 10);
        scores_lvl_3.bound(ty, 0, 9);
        scores_lvl_3.bound(n, 0, 3);

        Func min_val_lvl_2, min_x_lvl_2, min_y_lvl_2;
        min_val_lvl_2(tx, ty, n) = cast<uint>(std::numeric_limits<int>::max());
        //min_val_lvl_2(tx, ty, n) = cast<uint16_t>(std::numeric_limits<int>::max());
        min_x_lvl_2(tx, ty, n) = 0;
        min_y_lvl_2(tx, ty, n) = 0;

        min_x_lvl_2.bound(tx, 0, 10);
        min_x_lvl_2.bound(ty, 0, 9);
        min_x_lvl_2.bound(n, 0, 3);

        min_y_lvl_2.bound(tx, 0, 10);
        min_y_lvl_2.bound(ty, 0, 9);
        min_y_lvl_2.bound(n, 0, 3);

        min_val_lvl_2.bound(tx, 0, 10);
        min_val_lvl_2.bound(ty, 0, 9);
        min_val_lvl_2.bound(n, 0, 3);

        // Update the minimum function and coordinates
        Expr new_min_lvl_2 = select(scores_lvl_2(tx, ty, r_search_lvl_2.x, r_search_lvl_2.y, n) < min_val_lvl_2(tx, ty, n), scores_lvl_2(tx, ty, r_search_lvl_2.x, r_search_lvl_2.y, n), min_val_lvl_2(tx, ty, n));
        Expr new_min_x_lvl_2 = select(scores_lvl_2(tx, ty, r_search_lvl_2.x, r_search_lvl_2.y, n) < min_val_lvl_2(tx, ty, n), r_search_lvl_2.x, min_x_lvl_2(tx, ty, n));
        Expr new_min_y_lvl_2 = select(scores_lvl_2(tx, ty, r_search_lvl_2.x, r_search_lvl_2.y, n) < min_val_lvl_2(tx, ty, n), r_search_lvl_2.y, min_y_lvl_2(tx, ty, n));

        min_val_lvl_2(tx, ty, n) = new_min_lvl_2;
        min_x_lvl_2(tx, ty, n) = new_min_x_lvl_2;
        min_y_lvl_2(tx, ty, n) = new_min_y_lvl_2;

        //Tuple min_coor_lvl_2 = argmin(scores_lvl_2(tx, ty, r_search_lvl_2.x, r_search_lvl_2.y, n));

       /* Func: alignPyramid[2]
        * dtype: i16
        * True range: [-4, 4] (worst case)
        * Consumer(s): coarse_offset_lvl_2
        */
        alignPyramid_y[2](tx, ty, n) = select(n == 0, i16(0), i16(min_y_lvl_2(tx, ty, n)) + coarse_offset_lvl_2_y(tx, ty, n));
        alignPyramid_x[2](tx, ty, n) = select(n == 0, i16(0), i16(min_x_lvl_2(tx, ty, n)) + coarse_offset_lvl_2_x(tx, ty, n));


        alignPyramid_y[2].bound(tx, 0, 10);
        alignPyramid_y[2].bound(ty, 0, 9);
        alignPyramid_y[2].bound(n, 0, 3);

        alignPyramid_x[2].bound(tx, 0, 10);
        alignPyramid_x[2].bound(ty, 0, 9);
        alignPyramid_x[2].bound(n, 0, 3);

        // alignPyramid[2](tx, ty, xy, n) = select(n == 0, i16(0),
        //             xy == 0, i16(min_x_lvl_2(tx, ty, n)) + coarse_offset_lvl_2_x(tx, ty, n),
        //             i16(min_y_lvl_2(tx, ty, n)) + coarse_offset_lvl_2_y(tx, ty, n));

        // alignPyramid[2].bound(tx, 0, 10);
        // alignPyramid[2].bound(ty, 0, 9);
        // // alignPyramid[2].bound(xy, 0, 1);
        // alignPyramid[2].bound(n, 0, 3);
        
        //alignPyramid[2].trace_stores();

        Func provisional_output;
        provisional_output(x, y, n) = alignPyramid_x[2](x, y, n);
        output(x, y, c) = u8(provisional_output(x, y, c));
        output.bound(c, 0, 3);
        output.bound(x, 0, 10);
        output.bound(y, 0, 9);


        /* ALIGN PYRAMID LEVEL 1*/
        //Var tx, ty, xy, n;
        Var x_s_lvl_1, y_s_lvl_1;
      
        RDom r_tile_lvl_1(0, T_SIZE, 0, T_SIZE);
        RDom r_search_lvl_1(-4, 9, -4, 9);
        
        //coarse_offset_lvl_1(tx, ty, xy, n) = 2 * i32(ceil(upsample_float_size_2_for_alignment(alignPyramid[2], upsample_flow_gauss_widths[1], upsample_flow_gauss_heights[1])(tx, ty, xy, n)));
        
       /* Func: coarse_offset_lvl_1
        * dtype: i16
        * True range: [-56, 56]
        * Consumer(s): alignPyramid[1]
        */
        Func coarse_offset_lvl_1_y, coarse_offset_lvl_1_x;
        coarse_offset_lvl_1_x(tx, ty, n) = i16(2 * upsample_u16_size_2_for_alignment(alignPyramid_x[2], upsample_flow_gauss_widths[1], upsample_flow_gauss_heights[1])(tx, ty, n));
        coarse_offset_lvl_1_y(tx, ty, n) = i16(2 * upsample_u16_size_2_for_alignment(alignPyramid_y[2], upsample_flow_gauss_widths[1], upsample_flow_gauss_heights[1])(tx, ty, n));

        Expr x_ref_lvl_1 = clamp(tx * T_SIZE + r_tile_lvl_1.x, 0, gauss_width[1]-1);
        Expr y_ref_lvl_1 = clamp(ty * T_SIZE + r_tile_lvl_1.y, 0, gauss_height[1]-1);

        Expr x_cmp_lvl_1 = clamp(tx * T_SIZE + r_tile_lvl_1.x + coarse_offset_lvl_1_x(tx, ty, n) + x_s_lvl_1, 0, gauss_width[1]-1);
        Expr y_cmp_lvl_1 = clamp(ty * T_SIZE + r_tile_lvl_1.y + coarse_offset_lvl_1_y(tx, ty, n) + y_s_lvl_1, 0, gauss_height[1]-1);

        // Expr x_cmp_lvl_1 = clamp(tx * T_SIZE + r_tile_lvl_1.x + x_s_lvl_1, 0, gauss_width[1]-1);
        // Expr y_cmp_lvl_1 = clamp(ty * T_SIZE + r_tile_lvl_1.y + y_s_lvl_1, 0, gauss_height[1]-1);

        ///Expr dist_lvl_1 = abs(i16(gPyramid[1](x_ref_lvl_1, y_ref_lvl_1, 0)) - i16(gPyramid[1](x_cmp_lvl_1, y_cmp_lvl_1, n))); 
        Expr dist_lvl_1 = abs(i16(gPyramid1_LUT(x_ref_lvl_1, y_ref_lvl_1, 0)) - i16(gPyramid1_LUT(x_cmp_lvl_1, y_cmp_lvl_1, n))); 

       /* Func: scores_lvl_1
        * dtype: u32
        * True range: [0, 261888] (worst case)
        * Consumer(s): alignPyramid[1]
        */
        Func scores_lvl_1;
        scores_lvl_1(tx, ty, x_s_lvl_1, y_s_lvl_1, n) = sum(u32(dist_lvl_1));
        //scores_lvl_1(tx, ty, x_s_lvl_1, y_s_lvl_1, n) = sum(u16(dist_lvl_1));
        //scores_lvl_1.trace_stores();

        Func min_val_lvl_1, min_x_lvl_1, min_y_lvl_1;
        min_val_lvl_1(tx, ty, n) = cast<uint>(std::numeric_limits<int>::max());
        //min_val_lvl_1(tx, ty, n) = cast<uint16_t>(std::numeric_limits<int>::max());
        min_x_lvl_1(tx, ty, n) = 0;
        min_y_lvl_1(tx, ty, n) = 0;

        min_x_lvl_1.bound(tx, 0, 20);
        min_x_lvl_1.bound(ty, 0, 18);
        min_x_lvl_1.bound(n, 0, 3);

        min_y_lvl_1.bound(tx, 0, 20);
        min_y_lvl_1.bound(ty, 0, 18);
        min_y_lvl_1.bound(n, 0, 3);

        min_val_lvl_1.bound(tx, 0, 20);
        min_val_lvl_1.bound(ty, 0, 18);
        min_val_lvl_1.bound(n, 0, 3);

        // Update the minimum function and coordinates
        Expr new_min_lvl_1 = select(scores_lvl_1(tx, ty, r_search_lvl_1.x, r_search_lvl_1.y, n) < min_val_lvl_1(tx, ty, n), scores_lvl_1(tx, ty, r_search_lvl_1.x, r_search_lvl_1.y, n), min_val_lvl_1(tx, ty, n));
        Expr new_min_x_lvl_1 = select(scores_lvl_1(tx, ty, r_search_lvl_1.x, r_search_lvl_1.y, n) < min_val_lvl_1(tx, ty, n), r_search_lvl_1.x, min_x_lvl_1(tx, ty, n));
        Expr new_min_y_lvl_1 = select(scores_lvl_1(tx, ty, r_search_lvl_1.x, r_search_lvl_1.y, n) < min_val_lvl_1(tx, ty, n), r_search_lvl_1.y, min_y_lvl_1(tx, ty, n));

        min_val_lvl_1(tx, ty, n) = new_min_lvl_1;
        min_x_lvl_1(tx, ty, n) = new_min_x_lvl_1;
        min_y_lvl_1(tx, ty, n) = new_min_y_lvl_1;

        //Tuple min_coor_lvl_1 = argmin(scores_lvl_1(tx, ty, r_search_lvl_1.x, r_search_lvl_1.y, n));

       /* Func: alignPyramid[1]
        * dtype: i16
        * True range: [-4, 4] (worst case)
        * Consumer(s): coarse_offset_lvl_1
        */
        alignPyramid_y[1](tx, ty, n) = select(n == 0, i16(0), i16(min_y_lvl_1(tx, ty, n)) + coarse_offset_lvl_1_y(tx, ty, n));
        alignPyramid_x[1](tx, ty, n) = select(n == 0, i16(0), i16(min_x_lvl_1(tx, ty, n)) + coarse_offset_lvl_1_x(tx, ty, n));


        alignPyramid_y[1].bound(tx, 0, 20);
        alignPyramid_y[1].bound(ty, 0, 18);
        alignPyramid_y[1].bound(n, 0, 3);

        alignPyramid_x[1].bound(tx, 0, 20);
        alignPyramid_x[1].bound(ty, 0, 18);
        alignPyramid_x[1].bound(n, 0, 3);


         /* ALIGN PYRAMID LEVEL 0*/
        //Var tx, ty, xy, n;
        Var x_s_lvl_0, y_s_lvl_0;
        
        RDom r_tile_lvl_0(0, T_SIZE, 0, T_SIZE);
        RDom r_search_lvl_0(-4, 9, -4, 9);
        
        //coarse_offset_lvl_0(tx, ty, xy, n) = 2 * i32(ceil(upsample_float_size_2_for_alignment(alignPyramid[1], upsample_flow_gauss_widths[0], upsample_flow_gauss_heights[0])(tx, ty, xy, n)));
              
       /* Func: coarse_offset_lvl_0
        * dtype: i16
        * True range: [-120, 120]
        * Consumer(s): alignPyramid[0]
        */

        Func coarse_offset_lvl_0_y, coarse_offset_lvl_0_x;
        coarse_offset_lvl_0_x(tx, ty, n) = i16(2 * upsample_u16_size_2_for_alignment(alignPyramid_x[1], upsample_flow_gauss_widths[0], upsample_flow_gauss_heights[0])(tx, ty, n));
        coarse_offset_lvl_0_y(tx, ty, n) = i16(2 * upsample_u16_size_2_for_alignment(alignPyramid_y[1], upsample_flow_gauss_widths[0], upsample_flow_gauss_heights[0])(tx, ty, n));

        Expr x_ref_lvl_0 = clamp(tx * T_SIZE + r_tile_lvl_0.x, 0, gauss_width[0]-1);
        Expr y_ref_lvl_0 = clamp(ty * T_SIZE + r_tile_lvl_0.y, 0, gauss_height[0]-1);

        Expr x_cmp_lvl_0 = clamp(tx * T_SIZE + r_tile_lvl_0.x + coarse_offset_lvl_0_x(tx, ty, n) + x_s_lvl_0, 0, gauss_width[0]-1);
        Expr y_cmp_lvl_0 = clamp(ty * T_SIZE + r_tile_lvl_0.y + coarse_offset_lvl_0_y(tx, ty, n) + y_s_lvl_0, 0, gauss_height[0]-1);

        // Expr x_cmp_lvl_0 = clamp(tx * T_SIZE + r_tile_lvl_0.x + x_s_lvl_0, 0, gauss_width[0]-1);
        // Expr y_cmp_lvl_0 = clamp(ty * T_SIZE + r_tile_lvl_0.y + y_s_lvl_0, 0, gauss_height[0]-1);

        // Expr dist_lvl_0 = abs(i16(gPyramid[0](x_ref_lvl_0, y_ref_lvl_0, 0)) - i16(gPyramid[0](x_cmp_lvl_0, y_cmp_lvl_0, n))); 
        Expr dist_lvl_0 = abs(i16(gPyramid0_LUT(x_ref_lvl_0, y_ref_lvl_0, 0)) - i16(gPyramid0_LUT(x_cmp_lvl_0, y_cmp_lvl_0, n))); 

       /* Func: scores_lvl_0
        * dtype: u32
        * True range: [0, 261888] (worst case)
        * Consumer(s): alignPyramid[0]
        */
        Func scores_lvl_0;
        scores_lvl_0(tx, ty, x_s_lvl_0, y_s_lvl_0, n) = sum(u32(dist_lvl_0));
        //scores_lvl_0(tx, ty, x_s_lvl_0, y_s_lvl_0, n) = sum(u16(dist_lvl_0));
        //scores_lvl_0.trace_stores();
        Func min_val_lvl_0, min_x_lvl_0, min_y_lvl_0;
        min_val_lvl_0(tx, ty, n) = cast<uint>(std::numeric_limits<int>::max());
        //min_val_lvl_0(tx, ty, n) = cast<uint16_t>(std::numeric_limits<int>::max());
        min_x_lvl_0(tx, ty, n) = 0;
        min_y_lvl_0(tx, ty, n) = 0;

        min_x_lvl_0.bound(tx, 0, 39);
        min_x_lvl_0.bound(ty, 0, 35);
        min_x_lvl_0.bound(n, 0, 3);

        min_y_lvl_0.bound(tx, 0, 39);
        min_y_lvl_0.bound(ty, 0, 35);
        min_y_lvl_0.bound(n, 0, 3);

        min_val_lvl_0.bound(tx, 0, 39);
        min_val_lvl_0.bound(ty, 0, 35);
        min_val_lvl_0.bound(n, 0, 3);

        // Update the minimum function and coordinates
        Expr new_min_lvl_0 = select(scores_lvl_0(tx, ty, r_search_lvl_0.x, r_search_lvl_0.y, n) < min_val_lvl_0(tx, ty, n), scores_lvl_0(tx, ty, r_search_lvl_0.x, r_search_lvl_0.y, n), min_val_lvl_0(tx, ty, n));
        Expr new_min_x_lvl_0 = select(scores_lvl_0(tx, ty, r_search_lvl_0.x, r_search_lvl_0.y, n) < min_val_lvl_0(tx, ty, n), r_search_lvl_0.x, min_x_lvl_0(tx, ty, n));
        Expr new_min_y_lvl_0 = select(scores_lvl_0(tx, ty, r_search_lvl_0.x, r_search_lvl_0.y, n) < min_val_lvl_0(tx, ty, n), r_search_lvl_0.y, min_y_lvl_0(tx, ty, n));

        min_val_lvl_0(tx, ty, n) = new_min_lvl_0;
        min_x_lvl_0(tx, ty, n) = new_min_x_lvl_0;
        min_y_lvl_0(tx, ty, n) = new_min_y_lvl_0;

        //Tuple min_coor_lvl_0 = argmin(scores_lvl_0(tx, ty, r_search_lvl_0.x, r_search_lvl_0.y, n));

       /* Func: alignPyramid[0]
        * dtype: i16
        * True range: [-4, 4] (worst case)
        * Consumer(s): coarse_offset_lvl_0
        */
        alignPyramid_y[0](tx, ty, n) = select(n == 0, i16(0), i16(min_y_lvl_0(tx, ty, n)) + coarse_offset_lvl_0_y(tx, ty, n));
        alignPyramid_x[0](tx, ty, n) = select(n == 0, i16(0), i16(min_x_lvl_0(tx, ty, n)) + coarse_offset_lvl_0_x(tx, ty, n));


        alignPyramid_y[0].bound(tx, 0, 39);
        alignPyramid_y[0].bound(ty, 0, 35);
        alignPyramid_y[0].bound(n, 0, 3);

        alignPyramid_x[0].bound(tx, 0, 39);
        alignPyramid_x[0].bound(ty, 0, 35);
        alignPyramid_x[0].bound(n, 0, 3);

        //alignPyramid[0].trace_stores();
        
      //  /* Func: align_output
      //   * dtype: i16
      //   * True range: [-124, 124] (worst case)
      //   * Consumer(s): dist calculation (in merge)
      //   */
      //   Func align_output;
      //   align_output(tx, ty, xy, n) = alignPyramid[0](tx, ty, xy, n);


        // Func provisional_output;
        // provisional_output(x, y, n) = align_output(x, y, 0, n);
        // output(x, y, c) = u8(provisional_output(x, y, c));
        // output.bound(c, 0, 3);
        // output.bound(x, 0, 39);
        // output.bound(y, 0, 35);


        // Func provisional_output;
        // provisional_output(x, y, n) = alignPyramid_x[0](x, y, n);
        // output(x, y, c) = u8(provisional_output(x, y, c));
        // output.bound(c, 0, 3);
        // output.bound(x, 0, 39);
        // output.bound(y, 0, 35);


     


        // Debugging line 


        // Expr max_tx = (input.width()/2)/(T_SIZE);
        // Expr max_ty = (input.height()/2)/(T_SIZE);
        // output(x, y, c) = u8(align_output(x, y, 0, c));
        /* 
         * END ALIGN STEP
         */



        // Schedule
        //Var xo("xo"), yo("yo"), xi("xi"), yi("yi"), outer("outer");

      if (get_target().has_feature(Target::CoreIR)) {

      } else if (get_target().has_feature(Target::Clockwork)) {
    
        const int output_x_size = 10;
        const int output_y_size = 9;


        output.bound(x, 0, output_x_size);
        output.bound(y, 0, output_y_size);

        provisional_output.in().compute_root();

        provisional_output.in().tile(x, y, xo, yo, xi, yi, output_x_size, output_y_size)
        //provisional_output.in().tile(tx, ty, xo, yo, xi, yi, output_x_size, output_y_size)
        //provisional_output.in().tile(tx, ty, xo, yo, xi, yi, output_x_size, output_y_size)
          //.reorder(c, xi, yi, xo, yo)
          .reorder(n, xi, yi, xo, yo)
          //.reorder(n, xi, yi, xo, yo)
          .hw_accelerate(xi, xo);


        provisional_output.tile(x, y, xo, yo, xi, yi, output_x_size, output_y_size)
        //provisional_output.tile(tx, ty, xo, yo, xi, yi, output_x_size, output_y_size)
        //provisional_output.tile(tx, ty, xo, yo, xi, yi, output_x_size, output_y_size)
          //.reorder(c, xi, yi, xo, yo);
          .reorder(n, xi, yi, xo, yo);
          //.reorder(n, xi, yi, xo, yo);
        provisional_output.compute_at(provisional_output.in(), xo);
        provisional_output.store_in(MemoryType::GLB);

        // alignPyramid[0].compute_at(provisional_output, xo);
        // alignPyramid[1].compute_at(provisional_output, xo);
        //alignPyramid[2].compute_at(provisional_output, xo);
        //alignPyramid[3].compute_at(provisional_output, xo);


        // alignPyramid_x[0].compute_at(provisional_output, xo);
        // alignPyramid_y[0].compute_at(provisional_output, xo);

        // alignPyramid_x[1].compute_at(provisional_output, xo);
        // alignPyramid_y[1].compute_at(provisional_output, xo);

        alignPyramid_x[2].compute_at(provisional_output, xo);
        alignPyramid_y[2].compute_at(provisional_output, xo);

        alignPyramid_x[3].compute_at(provisional_output, xo);
        alignPyramid_y[3].compute_at(provisional_output, xo);
        

    
        hw_input_copy.compute_at(provisional_output, xo);
  
        hw_input.in().in().compute_at(provisional_output, xo); // represents the mem tile

        hw_input.in().compute_at(provisional_output.in(), xo); // represents the glb level
        hw_input.in().store_in(MemoryType::GLB);

        hw_input.bound(x, 0, 3);
        hw_input.bound(y, 0, 3);
        //hw_input.bound(xy, 0, 2);
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
    //Var x, y, tx, ty, xy, xi, yi, c, n;


    Func upsample_u16_size_2_for_alignment(Func f_in, Expr gauss_width, Expr gauss_height) {
      Var tx, ty, n;
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
      // // up.bound(xy, 0, 1);
      // // up.bound(n, 0, 3);

      up(tx, ty, n) = f_in(tx/2, ty/2, n);


      return up;
    }

    Func combined_upsample_u16_size_2_for_alignment(Func f_in, Expr gauss_width, Expr gauss_height) {
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
      // // up.bound(xy, 0, 1);
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