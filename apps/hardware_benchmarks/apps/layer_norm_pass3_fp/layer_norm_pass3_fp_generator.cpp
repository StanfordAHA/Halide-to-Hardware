#include "Halide.h"

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

class LayerNormPass3 : public Halide::Generator<LayerNormPass3> {
public:
    Input<Buffer<uint16_t>> input{ "input", 2 };
    Input<Buffer<uint16_t>> weight{ "weight", 2 };
    Input<Buffer<uint16_t>> bias{ "bias", 2 };
    Output<Buffer<uint16_t>> output{ "output", 2 };

    GeneratorParam<int> vec_width{ "vec_width", 384 };
    GeneratorParam<int> vec_height{ "vec_height", 128 };
    GeneratorParam<int> glb_i{ "glb_i", 16 };

    void generate() {
        /* THE ALGORITHM */
        // (input - wrong_beta) * (1/wrong_gamma) * weight + bias

        const float wrong_gamma = 1.2f;
        const float wrong_beta = -0.35f;

        Var x("x"), y("y");
        Func hw_input("hw_input"), input_host("input_host"), input_glb("input_glb"), input_cgra("input_cgra");
        Func hw_weight("hw_weight"), weight_host("weight_host"), weight_glb("weight_glb"), weight_cgra("weight_cgra");
        Func hw_bias("hw_bias"), bias_host("bias_host"), bias_glb("bias_glb"), bias_cgra("bias_cgra");
        Func hw_output("hw_output"), output_glb("output_glb"), output_cgra("output_cgra"), sum_cgra("sum_cgra");

        hw_input(x, y) = bf16(input(x, y));
        input_host(x, y) = hw_input(x, y);
        input_glb(x, y) = input_host(x, y);
        input_cgra(x, y) = input_glb(x, y);

        hw_weight(x, y) = bf16(weight(x, y));
        weight_host(x, y) = hw_weight(x, y);
        weight_glb(x, y) = weight_host(x, y);
        weight_cgra(x, y) = weight_glb(x, y);

        hw_bias(x, y) = bf16(bias(x, y));
        bias_host(x, y) = hw_bias(x, y);
        bias_glb(x, y) = bias_host(x, y);
        bias_cgra(x, y) = bias_glb(x, y);

        output_cgra(x, y) = (input_cgra(x, y) - bf16(wrong_beta)) * bf16(1.0f / wrong_gamma) * bf16(weight_cgra(x, y)) + bf16(bias_cgra(x, y));

        output_glb(x, y) = output_cgra(x, y);
        hw_output(x, y) = output_glb(x, y);
        output(x, y) = u16(hw_output(x, y));

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::Clockwork)) {

            input_host.bound(x, 0, vec_width).bound(y, 0, vec_height);
            input_glb.bound(x, 0, vec_width).bound(y, 0, vec_height);
            input_cgra.bound_extent(x, vec_width);

            weight_host.bound(x, 0, vec_width).bound(y, 0, vec_height);
            weight_glb.bound(x, 0, vec_width).bound(y, 0, vec_height);
            weight_cgra.bound_extent(x, vec_width);

            bias_host.bound(x, 0, vec_width).bound(y, 0, vec_height);
            bias_glb.bound(x, 0, vec_width).bound(y, 0, vec_height);
            bias_cgra.bound_extent(x, vec_width);

            output.bound(x, 0, vec_width).bound(y, 0, vec_height);
            hw_output.bound(x, 0, vec_width).bound(y, 0, vec_height);
            output_cgra.bound_extent(x, vec_width);

            Var y_host, y_glb, y_cgra;
            Var x_host, x_glb, x_cgra;

            // Produce loop levels: host, global buffer, cgra
            // L3 loop level
            hw_output.compute_root();
            hw_output
                .split(x, x_host, x_glb, vec_width)
                .split(y, y_host, y_glb, vec_height)
                .reorder(x_glb, y_glb, x_host, y_host)
                .hw_accelerate(y_glb, y_host)
                .unroll(x_glb, glb_i);

            // L2 loop level
            output_glb.compute_at(hw_output, y_host);
            output_glb
                .split(x, x_glb, x_cgra, vec_width)
                .split(y, y_glb, y_cgra, vec_height)
                .reorder(x_cgra, y_cgra, x_glb, y_glb)
                .unroll(x_cgra, glb_i);

            // L1 loop level
            output_cgra.compute_at(output_glb, y_glb).unroll(x, glb_i);


            // Input streaming
            input_host.compute_root().accelerator_input();
            input_glb.compute_at(hw_output, y_host).unroll(x, glb_i);
            input_cgra.compute_at(output_glb, y_glb).unroll(x, glb_i);

            // Weight streaming
            weight_host.compute_root().accelerator_input();
            weight_glb.compute_at(hw_output, y_host).unroll(x, glb_i);
            weight_cgra.compute_at(output_glb, y_glb).unroll(x, glb_i);

            // Bias streaming
            bias_host.compute_root().accelerator_input();
            bias_glb.compute_at(hw_output, y_host).unroll(x, glb_i);
            bias_cgra.compute_at(output_glb, y_glb).unroll(x, glb_i);

        } else {  // schedule to CPU
            output_cgra.compute_root().unroll(x, glb_i);
        }
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(LayerNormPass3, layer_norm_pass3_fp)
