#include "Halide.h"

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

class AvgPool : public Halide::Generator<AvgPool> {
public:
    Input<Buffer<uint16_t>> input{ "input", 3 };
    Output<Buffer<uint16_t>> output{ "output", 3 };

    // Ensure n_ic_fake // glb_i is 16 for graph generation, too small may create shift regs; too large may create MEMs
    GeneratorParam<int> in_img_fake{ "in_img_fake", 7 };
    GeneratorParam<int> n_ic_fake{ "n_ic_fake", 512 };

    GeneratorParam<int> in_img{ "in_img", 7 };
    GeneratorParam<int> n_ic{ "n_ic", 512 };

    // glb_i determines the input glb unrolling
    GeneratorParam<int> glb_i{ "glb_i", 32 };

    // glb_o determines the output glb unrolling
    GeneratorParam<int> glb_o{ "glb_o", 32 };

    void generate() {
        /* THE ALGORITHM */
        Var c("c"), x("x"), y("y");
        Func hw_input("hw_input");
        Func input_host("input_host"), input_glb("input_glb"), input_cgra("input_cgra");
        Func hw_output("hw_output"), output_glb("output_glb"), output_cgra("output_cgra");

        hw_input(c, x, y) = bf16(input(c, x, y));
        input_host(c, x, y) = hw_input(c, x, y);
        input_glb(c, x, y) = input_host(c, x, y);
        input_cgra(c, x, y) = input_glb(c, x, y);

        // Reduction over x and y
        RDom r(0, in_img_fake, 0, in_img_fake);
        output_cgra(c, x, y) = bf16(0);
        output_cgra(c, x, y) += input_cgra(c, x + r.x, y + r.y) * bf16(1.0f / (float(in_img) * float(in_img)));

        output_glb(c, x, y) = output_cgra(c, x, y);
        hw_output(c, x, y) = output_glb(c, x, y);
        output(c, x, y) = u16(hw_output(c, x, y));

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::Clockwork)) {

            input_host.bound(c, 0, n_ic_fake)
                .bound(x, 0, in_img_fake)
                .bound(y, 0, in_img_fake);
            input_glb.bound(c, 0, n_ic_fake)
                .bound(x, 0, in_img_fake)
                .bound(y, 0, in_img_fake);
            input_cgra.bound_extent(c, n_ic_fake);

            output.bound(c, 0, n_ic_fake).bound(x, 0, 1).bound(y, 0, 1);
            hw_output.bound(c, 0, n_ic_fake).bound(x, 0, 1).bound(y, 0, 1);
            output_glb.bound(c, 0, n_ic_fake).bound(x, 0, 1).bound(y, 0, 1);
            output_cgra.bound_extent(c, n_ic_fake);

            // Host-level tiling over channels
            Var c_host, c_glb, c_cgra;
            hw_output.compute_root()
                .split(c, c_host, c_glb, glb_o)
                .reorder(c_glb, c_host)
                .hw_accelerate(c_glb, c_host);

            // Global-buffer level
            output_glb.compute_at(hw_output, c_host)
                .split(c, c_glb, c_cgra, n_ic_fake)
                .reorder(c_cgra, c_glb);

            // Unroll output over glb
            hw_output.unroll(c_glb, glb_o);
            output_glb.unroll(c_cgra, glb_o);

            // CGRA-level compute & reduction scheduling
            output_cgra.compute_at(output_glb, c_glb)
                .update()
                .reorder(c, r.x, r.y, x, y)
                .unroll(c, glb_o);

            // Buffering and unrolling inputs
            input_host.compute_root().accelerator_input();
            input_glb.compute_at(hw_output, c_host).unroll(c, glb_i);

        } else {  // schedule to CPU
            output_cgra.compute_root();
        }
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(AvgPool, avgpool_layer_fp)
