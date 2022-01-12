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

    void generate() {
        /* THE ALGORITHM */

        // This implements the basic description of non-local means found at
        // https://en.wikipedia.org/wiki/Non-local_means

        Var x("x"), y("y"), c("c");

        //Expr inv_sigma_sq = -1.0f / (sigma * sigma * patch_size * patch_size);
        //Expr inv_sigma_sq = -69 / (patch_size * patch_size);

        // Add a boundary condition
        Func hw_input, hw_input_bfloat;
        Func repeated = BoundaryConditions::repeat_edge(input);
        //hw_input(x, y, c) = cast<float>(repeated(x, y, c));
        hw_input(x, y, c) = u16(repeated(x, y, c));
        hw_input_bfloat(x, y, c) = cast<bfloat16_t>(hw_input(x, y, c));

        // Define the difference images
        Var dx("dx"), dy("dy");
        Func dc("dc");
        Expr diff = hw_input_bfloat(x, y, c) - hw_input_bfloat(x + dx, y + dy, c);
        dc(x, y, dx, dy, c) = diff * diff;

        // Sum across color channels
        RDom channels(0, 3);
        Func d("d");
        d(x, y, dx, dy) = bfloat16_t(0);
        d(x, y, dx, dy) += (dc(x, y, dx, dy, channels));

        // Find the patch differences by blurring the difference images
        RDom patch_dom(-(7 / 2), 7);
        Func blur_d_y("blur_d_y");
        blur_d_y(x, y, dx, dy) = bfloat16_t(0);
        blur_d_y(x, y, dx, dy) += (d(x, y + patch_dom, dx, dy));

        Func blur_d("blur_d");
        blur_d(x, y, dx, dy) = bfloat16_t(0);
        blur_d(x, y, dx, dy) += (blur_d_y(x + patch_dom, y, dx, dy));

        // Compute the weights from the patch differences
        Func w("w");
        w(x, y, dx, dy) = exp(blur_d(x, y, dx, dy)) * bfloat16_t(-3 / 2);
        //w(x, y, dx, dy) = abs(blur_d(x, y, dx, dy) * -3 / 2);

        // Add an alpha channel
        Func clamped_with_alpha("clamped_with_alpha");
        clamped_with_alpha(x, y, c) = select(c==0, hw_input_bfloat(x, y, 0),
                                             c==1, hw_input_bfloat(x, y, 1),
                                             c==2, hw_input_bfloat(x, y, 2),
                                             /*c==3*/ bfloat16_t(1));

        // Define a reduction domain for the search area
        //RDom s_dom(-(search_area / 2), search_area, -(search_area / 2), search_area);
        RDom s_dom(-(7 / 2), 7, -(7 / 2), 7);

        // Compute the sum of the pixels in the search area
        Func non_local_means_sum("non_local_means_sum"), non_local_means, hw_output;
        non_local_means_sum(x, y, c) = bfloat16_t(0);
        non_local_means_sum(x, y, c) += cast<bfloat16_t>(w(x, y, s_dom.x, s_dom.y) * clamped_with_alpha(x + s_dom.x, y + s_dom.y, c));

        non_local_means(x, y, c) =
          clamp(non_local_means_sum(x, y, c) / non_local_means_sum(x, y, 3), bfloat16_t(0.0f), bfloat16_t(1.0f));

        hw_output(x, y, c) = u16(non_local_means(x, y, c));
        output(x, y, c) = u8(hw_output(x, y, c));

        /* THE SCHEDULE */

        Var tx("tx"), ty("ty"), xi("xi"), yi("yi");


        if (auto_schedule) {
            // nothing
        } else if (get_target().has_feature(Target::Clockwork)) {
          int outputSize = 64;
          int tileSize = 64;
            
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

          d.compute_at(hw_output, xo);
          blur_d_y.compute_at(hw_output, xo);
          blur_d.compute_at(hw_output, xo);
          non_local_means_sum.compute_at(hw_output, xo).reorder(c, x, y);
          non_local_means_sum.update().reorder(c, x, y);

          hw_input.in().reorder(c, x, y);
          
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

            const int vec = natural_vector_size<float>();

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
        }
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(NonLocalMeans, nlmeans)
