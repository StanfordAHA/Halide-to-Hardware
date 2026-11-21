#include "Halide.h"

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

// Four-tap causal depthwise Conv1D with SiLU on one [length, channels] L2 tile:
//   y(c, t) = bias(c) + sum_k weight(k, c) * input(c, t + k - 3),  output = y * sigmoid(y)
// x is the channel and y the sequence position, so the tile stays position-major.
// The Strait hook replaces the CGRA graph with depthwise_conv1d_silu_bf16 and
// sets the weight [4, channels] and bias [channels] shapes; this algorithm only
// fixes the IO names. process.cpp holds the reference.
class DepthwiseConv1dSilu : public Halide::Generator<DepthwiseConv1dSilu> {
public:
    Input<Buffer<uint16_t>> input{ "input", 2 };
    Input<Buffer<uint16_t>> weight{ "weight", 2 };
    Input<Buffer<uint16_t>> bias{ "bias", 2 };
    Output<Buffer<uint16_t>> output{ "output", 2 };

    GeneratorParam<int> vec_width{ "vec_width", 8 };
    GeneratorParam<int> vec_height{ "vec_height", 1024 };
    GeneratorParam<int> glb_i{ "glb_i", 8 };

    void generate() {
        /* THE ALGORITHM */
        Var x("x"), y("y");
        Func hw_input("hw_input"), input_host("input_host"), input_glb("input_glb"), input_cgra("input_cgra");
        Func hw_weight("hw_weight"), weight_host("weight_host"), weight_glb("weight_glb"), weight_cgra("weight_cgra");
        Func hw_bias("hw_bias"), bias_host("bias_host"), bias_glb("bias_glb"), bias_cgra("bias_cgra");
        Func hw_output("hw_output"), output_glb("output_glb"), output_cgra("output_cgra");

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

        Expr conv = f32(input_cgra(x, y)) * f32(weight_cgra(x, y)) + f32(bias_cgra(x, y));
        output_cgra(x, y) = bf16(conv / (1.0f + exp(-conv)));

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
            output_glb.bound(x, 0, vec_width).bound(y, 0, vec_height);
            output_cgra.bound_extent(x, vec_width).bound(y, 0, vec_height);

            Var y_host, y_glb, y_cgra;
            Var x_host, x_glb, x_cgra;

            // Host loop level
            hw_output.compute_root();
            hw_output
                .split(x, x_host, x_glb, vec_width)
                .split(y, y_host, y_glb, vec_height)
                .reorder(x_glb, y_glb, x_host, y_host)
                .hw_accelerate(y_glb, y_host);
            hw_output.unroll(x_glb, 1);

            // GLB loop level
            output_glb.compute_at(hw_output, y_host);
            output_glb
                .split(x, x_glb, x_cgra, vec_width)
                .split(y, y_glb, y_cgra, vec_height)
                .reorder(x_cgra, y_cgra, x_glb, y_glb);
            output_glb.unroll(x_cgra, 1);

            output_cgra.compute_at(output_glb, y_glb).unroll(x, 1);

            // Input buffers
            input_host.compute_root().accelerator_input();
            input_glb.compute_at(hw_output, y_host).unroll(x, 1);
            input_cgra.compute_at(output_glb, y_glb)
                    .split(x, x_glb, x_cgra, vec_width)
                    .split(y, y_glb, y_cgra, vec_height)
                    .reorder(x_cgra, y_cgra, x_glb, y_glb)
                    .unroll(x_cgra, 1);

            weight_host.compute_root().accelerator_input();
            weight_glb.compute_at(hw_output, y_host).unroll(x, 1);
            weight_cgra.compute_at(output_glb, y_glb)
                    .split(x, x_glb, x_cgra, vec_width)
                    .split(y, y_glb, y_cgra, vec_height)
                    .reorder(x_cgra, y_cgra, x_glb, y_glb)
                    .unroll(x_cgra, 1);

            bias_host.compute_root().accelerator_input();
            bias_glb.compute_at(hw_output, y_host).unroll(x, 1);
            bias_cgra.compute_at(output_glb, y_glb)
                    .split(x, x_glb, x_cgra, vec_width)
                    .split(y, y_glb, y_cgra, vec_height)
                    .reorder(x_cgra, y_cgra, x_glb, y_glb)
                    .unroll(x_cgra, 1);

        } else {  // schedule to CPU
            output_cgra.compute_root();
        }
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(DepthwiseConv1dSilu, depthwise_conv1d_silu_fp)
