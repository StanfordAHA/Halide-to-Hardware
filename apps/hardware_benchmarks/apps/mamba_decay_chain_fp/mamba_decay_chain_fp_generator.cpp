#include "Halide.h"

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

class MambaDecayChain : public Halide::Generator<MambaDecayChain> {
public:
    Input<Buffer<uint16_t>> a{ "a", 2 };
    Input<Buffer<uint16_t>> mask_strict{ "mask_strict", 2 };
    Input<Buffer<uint16_t>> mask_lower{ "mask_lower", 2 };
    Output<Buffer<uint16_t>> output{ "output", 2 };

    GeneratorParam<int> vec_width{ "vec_width", 128 };
    GeneratorParam<int> vec_height{ "vec_height", 1024 };
    GeneratorParam<int> glb_i{ "glb_i", 16 };

    void generate() {
        /* THE ALGORITHM */
        // Mamba2 decay = exp(cumsum_rows(a * strict mask)) * lower mask (mul_8, cumsum_1, exp, mul_9). Only the IO names
        // and shapes matter here: the strait template masked_col_cumsum_exp_bf16 replaces the graph (one a lane, column
        // scan with a Pond, exp from a ROM).

        Var x("x"), y("y");
        Func hw_a("hw_a"), a_host("a_host"), a_glb("a_glb"), a_cgra("a_cgra");
        Func hw_mask_strict("hw_mask_strict"), mask_strict_host("mask_strict_host"), mask_strict_glb("mask_strict_glb"), mask_strict_cgra("mask_strict_cgra");
        Func hw_mask_lower("hw_mask_lower"), mask_lower_host("mask_lower_host"), mask_lower_glb("mask_lower_glb"), mask_lower_cgra("mask_lower_cgra");
        Func hw_output("hw_output"), output_glb("output_glb"), output_cgra("output_cgra");

        hw_a(x, y) = bf16(a(x, y));
        a_host(x, y) = hw_a(x, y);
        a_glb(x, y) = a_host(x, y);
        a_cgra(x, y) = a_glb(x, y);

        hw_mask_strict(x, y) = bf16(mask_strict(x, y));
        mask_strict_host(x, y) = hw_mask_strict(x, y);
        mask_strict_glb(x, y) = mask_strict_host(x, y);
        mask_strict_cgra(x, y) = mask_strict_glb(x, y);

        hw_mask_lower(x, y) = bf16(mask_lower(x, y));
        mask_lower_host(x, y) = hw_mask_lower(x, y);
        mask_lower_glb(x, y) = mask_lower_host(x, y);
        mask_lower_cgra(x, y) = mask_lower_glb(x, y);

        output_cgra(x, y) = a_cgra(x, y) * mask_strict_cgra(x, y) * mask_lower_cgra(x, y);

        output_glb(x, y) = output_cgra(x, y);
        hw_output(x, y) = output_glb(x, y);
        output(x, y) = u16(hw_output(x, y));

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::Clockwork)) {

            a_host.bound(x, 0, vec_width).bound(y, 0, vec_height);
            a_glb.bound(x, 0, vec_width).bound(y, 0, vec_height);
            a_cgra.bound_extent(x, vec_width);

            mask_strict_host.bound(x, 0, vec_width).bound(y, 0, vec_height);
            mask_strict_glb.bound(x, 0, vec_width).bound(y, 0, vec_height);
            mask_strict_cgra.bound_extent(x, vec_width);

            mask_lower_host.bound(x, 0, vec_width).bound(y, 0, vec_height);
            mask_lower_glb.bound(x, 0, vec_width).bound(y, 0, vec_height);
            mask_lower_cgra.bound_extent(x, vec_width);

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
                .unroll(x_glb, 1);

            // L2 loop level
            output_glb.compute_at(hw_output, y_host);
            output_glb
                .split(x, x_glb, x_cgra, vec_width)
                .split(y, y_glb, y_cgra, vec_height)
                .reorder(x_cgra, y_cgra, x_glb, y_glb)
                .unroll(x_cgra, 1);

            // L1 loop level
            output_cgra.compute_at(output_glb, y_glb).unroll(x, 1);

            // a streaming
            a_host.compute_root().accelerator_input();
            a_glb.compute_at(hw_output, y_host).unroll(x, 1);
            a_cgra.compute_at(output_glb, y_glb).unroll(x, 1);

            // mask_strict streaming
            mask_strict_host.compute_root().accelerator_input();
            mask_strict_glb.compute_at(hw_output, y_host).unroll(x, 1);
            mask_strict_cgra.compute_at(output_glb, y_glb).unroll(x, 1);

            // mask_lower streaming
            mask_lower_host.compute_root().accelerator_input();
            mask_lower_glb.compute_at(hw_output, y_host).unroll(x, 1);
            mask_lower_cgra.compute_at(output_glb, y_glb).unroll(x, 1);

        } else {  // schedule to CPU
            output_cgra.compute_root().unroll(x, glb_i);
        }
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(MambaDecayChain, mamba_decay_chain_fp)
