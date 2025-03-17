#include "Halide.h"

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

// This app computes GELU with approximation x*sigmoid(1.702*x)
// GELU pass 2: x/(1+exp(-1.702*x))
class GELULayerPass2 : public Halide::Generator<GELULayerPass2> {
public:
    Input<Buffer<uint16_t>> input{ "input", 1 };
    Input<Buffer<uint16_t>> pass1_out{ "pass1_out", 1 };
    Output<Buffer<uint16_t>> output{ "output", 1 };

    GeneratorParam<int> vec_len{ "vec_len", 512 };

    // glb_i determines the input glb unrolling
    GeneratorParam<int> glb_i{ "glb_i", 8 };

    // glb_o determines the output glb unrolling
    GeneratorParam<int> glb_o{ "glb_o", 8 };

    void generate() {
        /* THE ALGORITHM */
        Var x("x");
        Func hw_input("hw_input"), input_host("input_host"), input_glb("input_glb"), input_cgra("input_cgra");
        Func hw_pass1_out("hw_pass1_out"), pass1_out_host("pass1_out_host"), pass1_out_glb("pass1_out_glb"), pass1_out_cgra("pass1_out_cgra");
        Func hw_output("hw_output"), output_glb("output_glb"), output_cgra("output_cgra");

        hw_input(x) = bf16(input(x));
        input_host(x) = hw_input(x);
        input_glb(x) = input_host(x);
        input_cgra(x) = input_glb(x);

        hw_pass1_out(x) = bf16(pass1_out(x));
        pass1_out_host(x) = hw_pass1_out(x);
        pass1_out_glb(x) = pass1_out_host(x);
        pass1_out_cgra(x) = pass1_out_glb(x);

        output_cgra(x) = input_cgra(x) / pass1_out_cgra(x);

        output_glb(x) = output_cgra(x);
        hw_output(x) = output_glb(x);
        output(x) = u16(hw_output(x));

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::Clockwork)) {

            output.bound(x, 0, vec_len);
            hw_output.bound(x, 0, vec_len);
            output_cgra.bound(x, 0, vec_len);

            Var x_host, x_glb, x_cgra;

            // Produce loop levels: host, global buffer, cgra
            // Host loop level
            hw_output.compute_root();
            hw_output
                .split(x, x_host, x_glb, vec_len)
                .reorder(x_glb, x_host)
                .hw_accelerate(x_glb, x_host);

            // GLB loop level
            output_glb.compute_at(hw_output, x_host);  // global buffer
            output_glb
                .split(x, x_glb, x_cgra, vec_len)
                .reorder(x_cgra, x_glb);

            // Unroll output over glb (default 1)
            hw_output.unroll(x_glb, glb_o);
            output_glb.unroll(x_cgra, glb_o);

            output_cgra.compute_at(output_glb, x_glb);  // memtile
            output_cgra.unroll(x, glb_i);

            // Input buffers
            input_host.compute_root().accelerator_input();
            input_glb.compute_at(hw_output, x_host).unroll(x, glb_i);
            input_cgra.compute_at(output_glb, x_glb).unroll(x, glb_i);

            pass1_out_host.compute_root().accelerator_input();
            pass1_out_glb.compute_at(hw_output, x_host).unroll(x, glb_i);
            pass1_out_cgra.compute_at(output_glb, x_glb).unroll(x, glb_i);

        } else {  // schedule to CPU
            output_cgra.compute_root();
        }
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(GELULayerPass2, gelu_pass2_fp)
