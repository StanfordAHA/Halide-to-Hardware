#include "Halide.h"

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

class NonLocalMeans : public Halide::Generator<NonLocalMeans> {
public:
    Input<Buffer<uint8_t>> input{"input", 3};
  //Input<int> patch_size{"patch_size"}; // default is 7
  //Input<int> search_area{"search_area"}; // default is 7
    //Input<float> sigma{"sigma"}; // default is 0.12

    Output<Buffer<uint8_t>> output{"output", 3};

  const int patch_size = 5;

    void generate() {
        /* THE ALGORITHM */

        // This implements the basic description of non-local means found at
        // https://en.wikipedia.org/wiki/Non-local_means

        Var x("x"), y("y"), c("c");

        //Expr inv_sigma_sq = -1.0f / (sigma * sigma * patch_size * patch_size);
        //Expr inv_sigma_sq = -69 / (patch_size * patch_size);
        Expr inv_sigma_sq = -1.0f / (0.12f * 0.12f * 7.f * 7.f);

        // Add a boundary condition
        Func hw_input, hw_input_bfloat, reordered;
        Func repeated = BoundaryConditions::repeat_edge(input);
        reordered(c, x, y) = repeated(x, y, c);
        //hw_input(x, y, c) = cast<float>(repeated(x, y, c));
        hw_input(c, x, y) = bf16(reordered(c, x, y)) / bf16(256);
        hw_input_bfloat(x, y, c) = hw_input(c, x, y);

        // Define the difference images
        Var dx("dx"), dy("dy");
        Func dc("dc");
        Expr diff = hw_input_bfloat(x, y, c) - hw_input_bfloat(x + dx, y + dy, c);
        dc(dx, dy, x, y, c) = diff * diff;

        // Sum across color channels
        RDom channels(0, 3);
        Func d("d");
        d(dx, dy, x, y) = bf16(0);
        d(dx, dy, x, y) += (dc(dx, dy, x, y, channels));

        // Find the patch differences by blurring the difference images
        RDom patch_dom(-(patch_size / 2), patch_size);
        Func blur_d_y("blur_d_y");
        blur_d_y(dx, dy, x, y) = bf16(0);
        blur_d_y(dx, dy, x, y) += (d(dx, dy, x, y + patch_dom));

        Func blur_d("blur_d");
        blur_d(dx, dy, x, y) = bf16(0);
        blur_d(dx, dy, x, y) += (blur_d_y(dx, dy, x + patch_dom, y));

        // Compute the weights from the patch differences
        Func w("w");
        w(x, y, dx, dy) = exp(blur_d(dx, dy, x, y) * bf16(inv_sigma_sq));
        //w(x, y, dx, dy) = (blur_d(x, y, dx, dy) * bf16(inv_sigma_sq));

        // Add an alpha channel
        Func clamped_with_alpha("clamped_with_alpha");
        clamped_with_alpha(x, y, c) = select(c==0, hw_input_bfloat(x, y, 0),
                                             c==1, hw_input_bfloat(x, y, 1),
                                             c==2, hw_input_bfloat(x, y, 2),
                                             /*c==3*/ bf16(1));

        // Define a reduction domain for the search area
        //RDom s_dom(-(search_area / 2), search_area, -(search_area / 2), search_area);
        RDom s_dom(-(patch_size / 2), patch_size, -(patch_size / 2), patch_size);

        // Compute the sum of the pixels in the search area
        Func non_local_means_sum("non_local_means_sum"), non_local_means, hw_output;
        non_local_means_sum(x, y, c) = bf16(0);
        non_local_means_sum(x, y, c) += bf16(w(x, y, s_dom.x, s_dom.y) * clamped_with_alpha(x + s_dom.x, y + s_dom.y, c));

        non_local_means(x, y, c) =
          clamp(non_local_means_sum(x, y, c) / non_local_means_sum(x, y, 3), bf16(0.0f), bf16(1.0f));
          //clamp(non_local_means_sum(x, y, c), bf16(0.0f), bf16(1.0f));

        hw_output(c, x, y) = non_local_means(x, y, c) * bf16(255);
        //hw_output(c, x, y) = hw_input_bfloat(x, y, c) * bf16(255);
        //hw_output(c, x, y) = hw_input(c, x, y) * bf16(255);
        //hw_output(c, x, y) = hw_input_bfloat(x, y, c, c) * bf16(255);
        output(x, y, c) = u8(hw_output(c, x, y));

        /* THE SCHEDULE */

        Var tx("tx"), ty("ty"), xi("xi"), yi("yi");


        if (auto_schedule) {
            // nothing
        } else if (get_target().has_feature(Target::Clockwork)) {
          int outputSize = 64;
          int tileSize = 32;
            
          output.bound(x, 0, outputSize);
          output.bound(y, 0, outputSize);
          output.bound(c, 0, 3);


          hw_output.compute_root();

          Var xi, xo, yi, yo;
          hw_output
            .tile(x, y, xo, yo, xi, yi, tileSize, tileSize)
            .reorder(c, xi, yi, xo, yo)
            .reorder_storage(c, x, y)
            .hw_accelerate(xi, xo);
        
          hw_output.unroll(c);

/*
          //d.compute_at(hw_output, xo).reorder(dy, x, y, dx).unroll(dy);
          d.compute_at(hw_output, xo);
          d.update().unroll(channels);
            //.reorder(dy, x, y, dx).unroll(dy);
          
          //blur_d_y.compute_at(hw_output, xo).reorder(dy, x, y, dx).unroll(dy);
          blur_d_y.compute_at(hw_output, xo);
          //blur_d_y.update().reorder(x, y, patch_dom, dx, dy);
          //blur_d_y.update().unroll(patch_dom).reorder(dy, x, y, dx).unroll(dy);
          blur_d_y.update().unroll(patch_dom);

          blur_d.compute_at(hw_output, xo).reorder(x, y, dx, dy);
          //blur_d.compute_at(hw_output, xo).reorder(dy, x, y, dx).unroll(dy);
          //blur_d.update().reorder(x, y, patch_dom, dx, dy);
          blur_d.update().reorder(patch_dom, x, y, dx, dy).unroll(patch_dom);
          //blur_d.update().reorder(patch_dom, dy, x, y, dx).unroll(patch_dom).unroll(dy);
          
          non_local_means_sum.compute_at(hw_output, xo).reorder(c, x, y).unroll(c);
          //non_local_means_sum.update().reorder(c, x, y, s_dom.x, s_dom.y).unroll(c);
          non_local_means_sum.update().reorder(c, x, y, s_dom.x, s_dom.y).unroll(c).unroll(s_dom.y).unroll(s_dom.x);
*/
          d.compute_at(hw_output, xo).reorder(dx, dy, x, y);
          d.update().reorder(dx, dy, x, y)
            .unroll(channels);
          
          blur_d_y.compute_at(hw_output, xo).reorder(dx, dy, x, y);
          //blur_d_y.update().reorder(x, y, patch_dom, dx, dy);
          blur_d_y.update().reorder(dx, dy, x, y)
            .unroll(patch_dom);
          
          blur_d.compute_at(hw_output, xo).reorder(dx, dy, x, y);
          //blur_d.update().reorder(x, y, patch_dom, dx, dy);
          blur_d.update().reorder(dx, dy, x, y)
            .unroll(patch_dom);
          
          non_local_means_sum.compute_at(hw_output, xo).reorder(c, x, y)
            .unroll(c);
          //non_local_means_sum.update().reorder(c, x, y, s_dom.x, s_dom.y);
          non_local_means_sum.update().reorder(c, x, y)
            .unroll(s_dom.x).unroll(s_dom.y)
            .unroll(c);

          hw_input.in().reorder(c, x, y).unroll(c);
          
          hw_input.reorder(c, x, y).stream_to_accelerator();

        } else if (get_target().has_gpu_feature()) {
            // 22 ms on a 2060 RTX
            Var xii, yii;

            // We'll use 32x16 thread blocks throughout. This was
            // found by just trying lots of sizes, but large thread
            // blocks are particularly good in the blur_d stage to
            // avoid doing wasted blurring work at tile boundaries
            // (especially for large patch sizes).

            non_local_means.compute_root()
                .reorder(c, x, y)
                .unroll(c)
                .gpu_tile(x, y, xi, yi, 32, 16);

            non_local_means_sum.compute_root()
                .gpu_tile(x, y, xi, yi, 32, 16)
                .update()
                .reorder(c, s_dom.x, x, y, s_dom.y)
                .tile(x, y, xi, yi, 32, 16)
                .gpu_blocks(x, y)
                .gpu_threads(xi, yi)
                .unroll(c);

            // The patch size we're benchmarking for is 7, which
            // implies an expansion of 6 pixels for footprint of the
            // blur, so we'll size tiles of blur_d to be a multiple of
            // the thread block size minus 6.
            blur_d.compute_at(non_local_means_sum, s_dom.y)
                .tile(x, y, xi, yi, 128 - 6, 32 - 6)
                .tile(xi, yi, xii, yii, 32, 16)
                .gpu_threads(xii, yii)
                .gpu_blocks(x, y, dx);

            blur_d_y.compute_at(blur_d, x)
                .tile(x, y, xi, yi, 32, 16)
                .gpu_threads(xi, yi);

            d.compute_at(blur_d, x)
                .tile(x, y, xi, yi, 32, 16)
                .gpu_threads(xi, yi);

        } else {
            // 64 ms on an Intel i9-9960X using 32 threads at 3.0 GHz

          int outputSize = 62;
          int tileSize = 62;
          //output.bound(x, 0, outputSize);
          //output.bound(y, 0, outputSize);
          output.bound(c, 0, 3);
            

          hw_output.compute_root();

          Var xi, xo, yi, yo;
          hw_output
            .tile(x, y, xo, yo, xi, yi, tileSize, tileSize)
            .reorder(c, xi, yi, xo, yo)
            .reorder_storage(c, x, y);
        
          hw_output.unroll(c);

          d.compute_at(hw_output, xo);
          d.update()
            .unroll(channels);
          
          blur_d_y.compute_at(hw_output, xo);
          //blur_d_y.update().reorder(x, y, patch_dom, dx, dy);
          blur_d_y.update()
            .unroll(patch_dom);
          
          blur_d.compute_at(hw_output, xo);
          //blur_d.update().reorder(x, y, patch_dom, dx, dy);
          blur_d.update()
            .unroll(patch_dom);
          
          non_local_means_sum.compute_at(hw_output, xo).reorder(c, x, y);
          //.unroll(c);
          //non_local_means_sum.update().reorder(c, x, y, s_dom.x, s_dom.y);
          non_local_means_sum.update().reorder(c, x, y);
          //.unroll(s_dom.x).unroll(s_dom.y).unroll(c);

          hw_input.in().reorder(c, x, y);

          //const int vec = natural_vector_size<float>();
          /*
            const int vec = 1;
            non_local_means.compute_root()
                .reorder(c, x, y)
                .tile(x, y, tx, ty, x, y, 16, 8)
                .parallel(ty)
                .vectorize(x, vec);
            blur_d_y.compute_at(non_local_means, tx)
                .reorder(y, x)
                .vectorize(x, vec);
            d.compute_at(non_local_means, tx)
                .vectorize(x, vec);
            non_local_means_sum.compute_at(non_local_means, x)
                .reorder(c, x, y)
                .bound(c, 0, 4)
                .unroll(c)
                .vectorize(x, vec);
            non_local_means_sum.update(0)
                .reorder(c, x, y, s_dom.x, s_dom.y)
                .unroll(c)
                .vectorize(x, vec);
            blur_d.compute_at(non_local_means_sum, x)
                .vectorize(x, vec);
          */

        }
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(NonLocalMeans, nlmeans)
