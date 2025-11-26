#include "Halide.h"

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

// This app computes GELU with approximation x*sigmoid(1.702*x)
// Compute GELU for the second half of lanes
class GELULayerPass2FP : public Halide::Generator<GELULayerPass2FP> {
public:
    Input<Buffer<uint16_t>> input{ "input", 2 };
    Output<Buffer<uint16_t>> output{ "output", 2 };

    GeneratorParam<int> vec_width{ "vec_width", 3072 };
    GeneratorParam<int> vec_height{ "vec_height", 64 };

    // glb_i determines the input glb unrolling
    GeneratorParam<int> glb_i{ "glb_i", 16 };

    void generate() {
        /* THE ALGORITHM */
        Var x("x"), y("y");
        Func hw_input("hw_input"), input_host("input_host"), input_glb("input_glb"), input_cgra("input_cgra");
        Func hw_output("hw_output"), output_glb("output_glb"), output_cgra("output_cgra");

        hw_input(x, y) = bf16(input(x, y));
        input_host(x, y) = hw_input(x, y);
        input_glb(x, y) = input_host(x, y);
        input_cgra(x, y) = input_glb(x, y);

        output_cgra(x, y) = input_cgra(x, y) / (bf16(1.0f) + exp(bf16(-1.702f) * input_cgra(x, y)));

        output_glb(x, y) = output_cgra(x, y);
        hw_output(x, y) = output_glb(x, y);
        output(x, y) = u16(hw_output(x, y));

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::Clockwork)) {

            output.bound(x, 0, vec_width).bound(y, 0, vec_height);
            hw_output.bound(x, 0, vec_width).bound(y, 0, vec_height);
            output_cgra.bound(x, 0, vec_width).bound(y, 0, vec_height);

            Var x_host, x_glb, x_cgra;
            Var y_host, y_glb, y_cgra;

            // Produce loop levels: host, global buffer, cgra
            // Host loop level
            hw_output.compute_root();
            hw_output
                .split(x, x_host, x_glb, vec_width)
                .split(y, y_host, y_glb, vec_height)
                .reorder(x_glb, y_glb, x_host, y_host)
                .hw_accelerate(y_glb, y_host)
                .unroll(x_glb, glb_i);

            // GLB loop level
            output_glb.compute_at(hw_output, y_host);  // global buffer
            output_glb
                .split(x, x_glb, x_cgra, vec_width)
                .split(y, y_glb, y_cgra, vec_height)
                .reorder(x_cgra, y_cgra, x_glb, y_glb)
                .unroll(x_cgra, glb_i);

            output_cgra.compute_at(output_glb, y_glb).unroll(x, glb_i);

            // Input buffers
            input_host.compute_root().accelerator_input();
            input_glb.compute_at(hw_output, y_host).unroll(x, glb_i);
            input_cgra
                .compute_at(output_glb, y_glb)
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

HALIDE_REGISTER_GENERATOR(GELULayerPass2FP, gelu_pass2_fp)
