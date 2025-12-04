#include "Halide.h"

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

// This app computes GELU with approximation x*sigmoid(1.702*x)
// First half lanes (e.g. 16 lanes) compute x/(1+exp(-1.702*x))
// Second half lanes (e.g. 16 lanes) pass through to GLB
class GELULayerPass1MuInputFP : public Halide::Generator<GELULayerPass1MuInputFP> {
public:
    Input<Buffer<uint16_t>> mu_input{ "mu_input", 2 };
    Output<Buffer<uint16_t>> output{ "output", 2 };

    GeneratorParam<int> vec_width{ "vec_width", 3072 };
    GeneratorParam<int> vec_height{ "vec_height", 64 };

    // mu_i determines the input glb unrolling
    GeneratorParam<int> mu_i{ "mu_i", 32 };

    // dummy_max_nop determines the number of nops to insert for path balancing
    GeneratorParam<int> dummy_max_nop{ "dummy_max_nop", 0 };

    void generate() {
        /* THE ALGORITHM */
        Var x("x"), y("y");
        Func mu_hw_input("mu_hw_input"), mu_input_host("mu_input_host"), mu_input_glb("mu_input_glb"), mu_input_cgra("mu_input_cgra");
        Func hw_output("hw_output"), output_glb("output_glb"), output_cgra("output_cgra");

        mu_hw_input(x, y) = bf16(mu_input(x, y));
        mu_input_host(x, y) = mu_hw_input(x, y);
        mu_input_glb(x, y) = mu_input_host(x, y);
        mu_input_cgra(x, y) = mu_input_glb(x, y);

        output_cgra(x, y) = mu_input_cgra(x, y) / (bf16(1.0f) + exp(bf16(-1.702f) * mu_input_cgra(x, y)));

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
                .unroll(x_glb, mu_i);

            // GLB loop level
            output_glb.compute_at(hw_output, y_host);  // global buffer
            output_glb
                .split(x, x_glb, x_cgra, vec_width)
                .split(y, y_glb, y_cgra, vec_height)
                .reorder(x_cgra, y_cgra, x_glb, y_glb)
                .unroll(x_cgra, mu_i);

            output_cgra.compute_at(output_glb, y_glb).unroll(x, mu_i);

            // Input buffers
            mu_input_host.compute_root().accelerator_input();
            mu_input_glb.compute_at(hw_output, y_host).unroll(x, mu_i);
            mu_input_cgra
                .compute_at(output_glb, y_glb)
                .split(x, x_glb, x_cgra, vec_width)
                .split(y, y_glb, y_cgra, vec_height)
                .reorder(x_cgra, y_cgra, x_glb, y_glb)
                .unroll(x_cgra, mu_i);

        } else {  // schedule to CPU
            output_cgra.compute_root();
        }
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(GELULayerPass1MuInputFP, gelu_pass1_mu_input_fp)
