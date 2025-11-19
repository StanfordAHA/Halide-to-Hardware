#include "Halide.h"

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

// This app does 1/sum(exp(x-max(x))), where max(x) is pre-loaded
class StableSoftmaxPass2 : public Halide::Generator<StableSoftmaxPass2> {
public:
    Input<Buffer<uint16_t>> input{ "input", 2 };
    Input<Buffer<uint16_t>> vec_max{ "vec_max", 2 };
    Output<Buffer<uint16_t>> output{ "output", 2 };

    // After hacking, the scheduling actually does reduction across the entire vec_height x vec_width array
    GeneratorParam<int> vec_height{ "vec_height", 2 };
    GeneratorParam<int> vec_width{ "vec_width", 256 };

    // glb_i determines the input glb unrolling
    GeneratorParam<int> glb_i{ "glb_i", 8 };

    void generate() {
        /* THE ALGORITHM */
        // Input: BF16 activation stored in 4 GLB Tiles
        // Vec max: BF16 max stored in 1 GLB Tile
        // Output: BF16 output stored in 4 GLB Tile
        // Output = exp(input - vec_max)
        Var x("x"), y("y");
        Func hw_input("hw_input");
        Func input_host("input_host"), input_glb("input_glb"), input_cgra("input_cgra");
        Func hw_vec_max("hw_vec_max");
        Func vec_max_host("vec_max_host"), vec_max_glb("vec_max_glb"), vec_max_cgra("vec_max_cgra");
        Func hw_output("hw_output"), output_glb("output_glb"), output_cgra("output_cgra");

        hw_input(x, y) = bf16(input(x, y));
        input_host(x, y) = hw_input(x, y);
        input_glb(x, y) = input_host(x, y);
        input_cgra(x, y) = input_glb(x, y);

        hw_vec_max(x, y) = bf16(vec_max(x, y));
        vec_max_host(x, y) = hw_vec_max(x, y);
        vec_max_glb(x, y) = vec_max_host(x, y);
        vec_max_cgra(x, y) = vec_max_glb(x, y);

        output_cgra(x, y) = exp(input_cgra(x, y) - vec_max_cgra(x, y));

        output_glb(x, y) = output_cgra(x, y);
        hw_output(x, y) = output_glb(x, y);
        output(x, y) = u16(hw_output(x, y));

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::Clockwork)) {
            input_host.bound(x, 0, vec_width).bound(y, 0, vec_height);
            input_glb.bound(x, 0, vec_width).bound(y, 0, vec_height);
            input_cgra.bound_extent(x, vec_width);

            vec_max_host.bound(x, 0, vec_width).bound(y, 0, vec_height);
            vec_max_glb.bound(x, 0, vec_width).bound(y, 0, vec_height);
            vec_max_cgra.bound_extent(x, vec_width);

            output.bound(x, 0, vec_width).bound(y, 0, vec_height);
            hw_output.bound(x, 0, vec_width).bound(y, 0, vec_height);
            output_glb.bound(x, 0, vec_width).bound(y, 0, vec_height);
            output_cgra.bound_extent(x, vec_width).bound(y, 0, vec_height);

            Var y_host, y_glb, y_cgra;
            Var x_host, x_glb, x_cgra;

            // Produce loop levels: host, global buffer, cgra
            // Host loop level
            hw_output.compute_root();
            hw_output
                .split(x, x_host, x_glb, vec_width)
                .split(y, y_host, y_glb, vec_height)
                .reorder(x_glb, y_glb, x_host, y_host)
                .hw_accelerate(y_glb, y_host);
            hw_output.unroll(x_glb, glb_i);

            // GLB loop level
            output_glb.compute_at(hw_output, y_host);  // global buffer
            output_glb
                .split(x, x_glb, x_cgra, vec_width)
                .split(y, y_glb, y_cgra, vec_height)
                .reorder(x_cgra, y_cgra, x_glb, y_glb);
            output_glb.unroll(x_cgra, glb_i);

            output_cgra.compute_at(output_glb, y_glb).unroll(x, glb_i);

            // Input buffers
            input_host.compute_root().accelerator_input();
            input_glb.compute_at(hw_output, y_host).unroll(x, glb_i);
            input_cgra.compute_at(output_glb, y_glb)
                    .split(x, x_glb, x_cgra, vec_width)
                    .split(y, y_glb, y_cgra, vec_height)
                    .reorder(x_cgra, y_cgra, x_glb, y_glb)
                    .unroll(x_cgra, glb_i);

            vec_max_host.compute_root().accelerator_input();
            vec_max_glb.compute_at(hw_output, y_host).unroll(x, glb_i);
            vec_max_cgra.compute_at(output_glb, y_glb)
                    .split(x, x_glb, x_cgra, vec_width)
                    .split(y, y_glb, y_cgra, vec_height)
                    .reorder(x_cgra, y_cgra, x_glb, y_glb)
                    .unroll(x_cgra, glb_i);

        } else {  // schedule to CPU
            output_cgra.compute_root();
        }
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(StableSoftmaxPass2, stable_softmax_pass2_fp)
