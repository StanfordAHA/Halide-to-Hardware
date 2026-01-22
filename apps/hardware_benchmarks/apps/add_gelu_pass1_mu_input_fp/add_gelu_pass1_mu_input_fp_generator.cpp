#include "Halide.h"

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

// This app computes PSUM ADD + GELU with approximation x*sigmoid(1.702*x)
// First half lanes (e.g. 16 lanes) compute Add and x/(1+exp(-1.702*x))
// Second half lanes (e.g. 16 lanes) do Add with PSUM0 in GLB
class AddGELULayerPass1MuInputFP : public Halide::Generator<AddGELULayerPass1MuInputFP> {
public:
    Input<Buffer<uint16_t>> mu_input{ "mu_input", 2 };
    Input<Buffer<uint16_t>> input_psum0{ "input_psum0", 2 };
    Output<Buffer<uint16_t>> output{ "output", 2 };

    GeneratorParam<int> vec_width{ "vec_width", 3072 };
    GeneratorParam<int> vec_height{ "vec_height", 64 };

    GeneratorParam<int> n_oc{"n_oc", 3072};

    // mu_i determines the input glb unrolling
    GeneratorParam<int> mu_i{ "mu_i", 32 };

    void generate() {
        /* THE ALGORITHM */
        Var x("x"), y("y");
        Func mu_hw_input("mu_hw_input"), mu_input_host("mu_input_host"), mu_input_glb("mu_input_glb"), mu_input_cgra("mu_input_cgra");
        Func hw_psum0_input("hw_psum0_input"), input_psum0_host("input_psum0_host"), input_psum0_glb("input_psum0_glb"), input_psum0_cgra("input_psum0_cgra");
        Func hw_add_gelu_upper_output("hw_add_gelu_upper_output"), output_add_gelu_upper_glb("output_add_gelu_upper_glb"), output_add_gelu_upper_cgra("output_add_gelu_upper_cgra");

        mu_hw_input(x, y) = bf16(mu_input(x, y));
        mu_input_host(x, y) = mu_hw_input(x, y);
        mu_input_glb(x, y) = mu_input_host(x, y);
        mu_input_cgra(x, y) = mu_input_glb(x, y);

        hw_psum0_input(x, y) = bf16(input_psum0(x, y));
        input_psum0_host(x, y) = hw_psum0_input(x, y);
        input_psum0_glb(x, y) = input_psum0_host(x, y);
        input_psum0_cgra(x, y) = input_psum0_glb(x, y);

        output_add_gelu_upper_cgra(x, y) =
            (mu_input_cgra(x, y) + input_psum0_cgra(x, y)) / (
                bf16(1.0f) + exp(
                    bf16(-1.702f) * (mu_input_cgra(x, y) + input_psum0_cgra(x, y))
                )
            );

        output_add_gelu_upper_glb(x, y) = output_add_gelu_upper_cgra(x, y);
        hw_add_gelu_upper_output(x, y) = output_add_gelu_upper_glb(x, y);
        output(x, y) = u16(hw_add_gelu_upper_output(x, y));

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::Clockwork)) {

            output.bound(x, 0, vec_width).bound(y, 0, vec_height);
            hw_add_gelu_upper_output.bound(x, 0, vec_width).bound(y, 0, vec_height);
            output_add_gelu_upper_cgra.bound(x, 0, vec_width).bound(y, 0, vec_height);

            Var x_host, x_glb, x_cgra;
            Var y_host, y_glb, y_cgra;

            // Produce loop levels: host, global buffer, cgra
            // Host loop level
            hw_add_gelu_upper_output.compute_root();
            hw_add_gelu_upper_output
                .split(x, x_host, x_glb, vec_width)
                .split(y, y_host, y_glb, vec_height)
                .reorder(x_glb, y_glb, x_host, y_host)
                .hw_accelerate(y_glb, y_host)
                .unroll(x_glb, mu_i);

            // GLB loop level
            output_add_gelu_upper_glb.compute_at(hw_add_gelu_upper_output, y_host);  // global buffer
            output_add_gelu_upper_glb
                .split(x, x_glb, x_cgra, vec_width)
                .split(y, y_glb, y_cgra, vec_height)
                .reorder(x_cgra, y_cgra, x_glb, y_glb)
                .unroll(x_cgra, mu_i);

            output_add_gelu_upper_cgra.compute_at(output_add_gelu_upper_glb, y_glb).unroll(x, mu_i);

            // Input buffers
            mu_input_host.compute_root().accelerator_input();
            mu_input_glb.compute_at(hw_add_gelu_upper_output, y_host).unroll(x, mu_i);
            mu_input_cgra
                .compute_at(output_add_gelu_upper_glb, y_glb)
                .split(x, x_glb, x_cgra, vec_width)
                .split(y, y_glb, y_cgra, vec_height)
                .reorder(x_cgra, y_cgra, x_glb, y_glb)
                .unroll(x_cgra, mu_i);

            input_psum0_host.compute_root().accelerator_input();
            input_psum0_glb.compute_at(hw_add_gelu_upper_output, y_host).unroll(x, mu_i);
            input_psum0_cgra
                .compute_at(output_add_gelu_upper_glb, y_glb)
                .split(x, x_glb, x_cgra, vec_width)
                .split(y, y_glb, y_cgra, vec_height)
                .reorder(x_cgra, y_cgra, x_glb, y_glb)
                .unroll(x_cgra, mu_i);

        } else {  // schedule to CPU
            output_add_gelu_upper_cgra.compute_root();
        }
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(AddGELULayerPass1MuInputFP, add_gelu_pass1_mu_input_fp)
