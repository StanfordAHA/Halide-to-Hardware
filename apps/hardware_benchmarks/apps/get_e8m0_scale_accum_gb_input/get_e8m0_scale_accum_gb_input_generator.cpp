#include "Halide.h"

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

class get_scale_accum : public Halide::Generator<get_scale_accum> {
public:
    Input<Buffer<uint16_t>> input{ "input", 2 };
    Output<Buffer<uint16_t>> output{ "output", 2 };

    // Ensure head_dim // glb_i is 32 for graph generation, too small may create shift regs; too large may create MEMs
    GeneratorParam<int> head_dim_fake{ "head_dim_fake", 64 };
    GeneratorParam<int> seq_heads_prod_fake{ "seq_heads_prod_fake", 128 };

    GeneratorParam<int> head_dim{ "head_dim", 64 };
    GeneratorParam<int> seq_heads_prod{ "seq_heads_prod", 128 };

    // glb_i determines the input glb unrolling
    GeneratorParam<int> glb_i{ "glb_i", 32 };

    void generate() {
        /* THE ALGORITHM */
        // Input: BF16 input from 32 software GLB IOs in 4 GLB Tiles
        // Output: Packed e8m0 scale stored in 2 GLB Tiles
        Var d("d"), s("s");
        Func hw_input("hw_input");
        Func input_host("input_host"), input_glb("input_glb"), input_cgra("input_cgra");
        Func hw_output("hw_output"), output_glb("output_glb"), output_cgra("output_cgra");
        Expr glb_o = int(glb_i) / 2;
        // Expr glb_o = int(glb_i);

        hw_input(d, s) = bf16(input(d, s));
        input_host(d, s) = hw_input(d, s);
        input_glb(d, s) = input_host(d, s);
        input_cgra(d, s) = input_glb(d, s);

        // Reduction over seq_heads_prod dimension with block size of 64
        const int block_size = 64;
        RDom r(0, block_size);
        Func abs_max;

        // abs_max reduction (but use fake adder for graph generation)
        abs_max(d, s) = bf16(0);
        abs_max(d, s) += input_cgra(d, s * block_size + r.x);

        // Data packing
        output_cgra(d, s) = bit8_pack(abs_max(2*d+1, s), abs_max(2*d, s));

        output_glb(d, s) = output_cgra(d, s);
        hw_output(d, s) = output_glb(d, s);
        output(d, s) = u16(hw_output(d, s));

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::Clockwork)) {

            input_host.bound(d, 0, head_dim_fake).bound(s, 0, seq_heads_prod_fake);
            input_glb.bound(d, 0, head_dim_fake).bound(s, 0, seq_heads_prod_fake);

            output.bound(d, 0, head_dim_fake / 2).bound(s, 0, seq_heads_prod_fake / block_size);
            hw_output.bound(d, 0, head_dim_fake / 2).bound(s, 0, seq_heads_prod_fake / block_size);
            output_glb.bound(d, 0, head_dim_fake / 2).bound(s, 0, seq_heads_prod_fake / block_size);
            output_cgra.bound(d, 0, head_dim_fake / 2).bound(s, 0, seq_heads_prod_fake / block_size);

            // output.bound(d, 0, head_dim_fake).bound(s, 0, seq_heads_prod_fake / block_size);
            // hw_output.bound(d, 0, head_dim_fake).bound(s, 0, seq_heads_prod_fake / block_size);
            // output_glb.bound(d, 0, head_dim_fake).bound(s, 0, seq_heads_prod_fake / block_size);
            // output_cgra.bound(d, 0, head_dim_fake).bound(s, 0, seq_heads_prod_fake / block_size);

            // Host-level tiling over channels
            Var d_host, d_glb, d_cgra;
            hw_output.compute_root()
                .split(d, d_host, d_glb, glb_o)
                .reorder(d_glb, d_host)
                .hw_accelerate(d_glb, d_host);

            // Global-buffer level
            output_glb.compute_at(hw_output, d_host)
                .split(d, d_glb, d_cgra, head_dim_fake / 2)
                .reorder(d_cgra, d_glb);

            // Unroll output over glb
            hw_output.unroll(d_glb, glb_o);
            output_glb.unroll(d_cgra, glb_o);

            // CGRA-level compute & reduction scheduling
            abs_max.compute_at(output_glb, d_glb);
            abs_max.update().reorder(d, r.x, s).unroll(d, glb_i);
            // output_cgra.compute_at(output_glb, s_glb).unroll(d, glb_o);
            output_cgra.compute_at(output_glb, d_glb);
            output_cgra.unroll(d, glb_o);

            // Buffering and unrolling inputs
            input_host.compute_root().accelerator_input();
            input_glb.compute_at(hw_output, d_host).unroll(d, glb_i);

        } else {  // schedule to CPU
            output_cgra.compute_root();
        }
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(get_scale_accum, get_e8m0_scale_accum_gb_input)
