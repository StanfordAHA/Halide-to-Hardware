#include "Halide.h"

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

class MambaXEnd : public Halide::Generator<MambaXEnd> {
public:
    Input<Buffer<uint16_t>> input{ "input", 2 };
    Input<Buffer<uint16_t>> dt{ "dt", 2 };
    Input<Buffer<uint16_t>> decay{ "decay", 2 };
    Output<Buffer<uint16_t>> output{ "output", 2 };

    GeneratorParam<int> vec_width{ "vec_width", 256 };
    GeneratorParam<int> vec_height{ "vec_height", 256 };
    GeneratorParam<int> glb_i{ "glb_i", 16 };

    void generate() {
        /* THE ALGORITHM */
        // Mamba2 X_end = (x * dt) * decay_to_end (mul_11). Only the IO names and shapes matter here: the strait
        // template elementwise_mul_head_gather_bf16 replaces the graph (dt and decay_to_end gather trees).

        Var x("x"), y("y");
        Func hw_input("hw_input"), input_host("input_host"), input_glb("input_glb"), input_cgra("input_cgra");
        Func hw_dt("hw_dt"), dt_host("dt_host"), dt_glb("dt_glb"), dt_cgra("dt_cgra");
        Func hw_decay("hw_decay"), decay_host("decay_host"), decay_glb("decay_glb"), decay_cgra("decay_cgra");
        Func hw_output("hw_output"), output_glb("output_glb"), output_cgra("output_cgra");

        hw_input(x, y) = bf16(input(x, y));
        input_host(x, y) = hw_input(x, y);
        input_glb(x, y) = input_host(x, y);
        input_cgra(x, y) = input_glb(x, y);

        hw_dt(x, y) = bf16(dt(x, y));
        dt_host(x, y) = hw_dt(x, y);
        dt_glb(x, y) = dt_host(x, y);
        dt_cgra(x, y) = dt_glb(x, y);

        hw_decay(x, y) = bf16(decay(x, y));
        decay_host(x, y) = hw_decay(x, y);
        decay_glb(x, y) = decay_host(x, y);
        decay_cgra(x, y) = decay_glb(x, y);

        output_cgra(x, y) = input_cgra(x, y) * dt_cgra(x, y) * decay_cgra(x, y);

        output_glb(x, y) = output_cgra(x, y);
        hw_output(x, y) = output_glb(x, y);
        output(x, y) = u16(hw_output(x, y));

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::Clockwork)) {

            input_host.bound(x, 0, vec_width).bound(y, 0, vec_height);
            input_glb.bound(x, 0, vec_width).bound(y, 0, vec_height);
            input_cgra.bound_extent(x, vec_width);

            dt_host.bound(x, 0, vec_width).bound(y, 0, vec_height);
            dt_glb.bound(x, 0, vec_width).bound(y, 0, vec_height);
            dt_cgra.bound_extent(x, vec_width);

            decay_host.bound(x, 0, vec_width).bound(y, 0, vec_height);
            decay_glb.bound(x, 0, vec_width).bound(y, 0, vec_height);
            decay_cgra.bound_extent(x, vec_width);

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


            // Input streaming
            input_host.compute_root().accelerator_input();
            input_glb.compute_at(hw_output, y_host).unroll(x, 1);
            input_cgra.compute_at(output_glb, y_glb).unroll(x, 1);

            // dt streaming
            dt_host.compute_root().accelerator_input();
            dt_glb.compute_at(hw_output, y_host).unroll(x, 1);
            dt_cgra.compute_at(output_glb, y_glb).unroll(x, 1);

            // decay_to_end streaming
            decay_host.compute_root().accelerator_input();
            decay_glb.compute_at(hw_output, y_host).unroll(x, 1);
            decay_cgra.compute_at(output_glb, y_glb).unroll(x, 1);

        } else {  // schedule to CPU
            output_cgra.compute_root().unroll(x, glb_i);
        }
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(MambaXEnd, mamba_x_end_fp)
