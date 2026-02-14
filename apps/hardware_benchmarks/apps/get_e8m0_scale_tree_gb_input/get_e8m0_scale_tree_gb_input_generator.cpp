#include "Halide.h"

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

class Get_E8M0_Scale_Tree : public Halide::Generator<Get_E8M0_Scale_Tree> {
public:
    // input: input from Global Buffer IOs
    // scale_output: unpacked e8m0 scale
    Input<Buffer<uint16_t>> input{ "input", 2 };
    Output<Buffer<uint16_t>> scale_output{ "scale_output", 1 };

    // e.g. 56x56 image input, y dim
    GeneratorParam<int> vec_height{ "vec_height", 3136 };
    // e.g. 32 channels, x dim
    GeneratorParam<int> vec_width_fake{ "vec_width_fake", 32 };
    GeneratorParam<int> vec_width{ "vec_width", 64 };

    // glb_i determines the glb input unrolling
    GeneratorParam<int> glb_i{ "glb_i", 32 };

    // glb_o determines the output glb unrolling
    GeneratorParam<int> glb_o{ "glb_o", 1 };

    // Fake to help aha flow run
    GeneratorParam<int> dim0_fake{ "dim0_fake", 3072 };

    // tree_stages determines the number of stages in the reduction tree: log2(glb_i)
    GeneratorParam<int> tree_stages{ "tree_stages", 5 };

    void generate() {
        /* THE ALGORITHM */
        // Input: BF16 input from 32 software GLB IOs in 4 GLB Tiles
        // Output: Unpacked e8m0 scale stored in 1 GLB Tile
        // Dimention variables
        Var x("x"), y("y");
        // GLB input buffers
        Func hw_input("hw_input"), input_host("input_host"), input_gb("input_gb"), input_cgra("input_cgra");
        // scale_output output buffers
        Func hw_scale_output("hw_scale_output"), scale_output_glb("scale_output_glb"), scale_output_cgra("scale_output_cgra");

        // GLB input buffers assignment
        hw_input(x, y) = bf16(input(x, y));
        input_host(x, y) = hw_input(x, y);
        input_gb(x, y) = input_host(x, y);
        input_cgra(x, y) = input_gb(x, y);

        // Define reduction trees
        const int tile_size = 1 << int(tree_stages);
        const int total_stages = int(tree_stages);
        std::vector<Func> tree_glb(total_stages + 1);
        std::vector<Func> tree_fifo(total_stages + 1);
        tree_glb[0](x, y) = input_cgra(x, y);
        tree_fifo[0](x, y) = input_cgra(x, y);

        // Reduction tree for GLB input
        for (int s = 1; s <= total_stages; s++) {
            Func stage("tree_glb_" + std::to_string(s));
            // At stage s the effective domain (along xi) is ceil(tile_size / (2^s)).
            Expr prev_extent = (tile_size + ((1 << (s - 1)) - 1)) / (1 << (s - 1));
            stage(x, y) = abs_max_bf16(
                tree_glb[s - 1](2 * x, y),
                select((2 * x + 1) < prev_extent, tree_glb[s - 1](2 * x + 1, y), 0));
            tree_glb[s] = stage;
        }

        // Reduction tree for memory input
        for (int s = 1; s <= total_stages; s++) {
            Func stage("tree_fifo_" + std::to_string(s));
            // At stage s the effective domain (along xi) is ceil(tile_size / (2^s)).
            Expr prev_extent = (tile_size + ((1 << (s - 1)) - 1)) / (1 << (s - 1));
            stage(x, y) = abs_max_bf16(
                tree_fifo[s - 1](2 * x, y),
                select((2 * x + 1) < prev_extent, tree_fifo[s - 1](2 * x + 1, y), 0));
            tree_fifo[s] = stage;
        }

        // get e8m0 scale
        scale_output_cgra(y) = get_shared_exp(abs_max_bf16(tree_glb[total_stages](0, y), tree_fifo[total_stages](0, y)));

        // output buffers assignment
        // scale_output_glb(y_pack) = pack_out(y_pack);
        scale_output_glb(y) = scale_output_cgra(y);
        hw_scale_output(y) = scale_output_glb(y);
        scale_output(y) = u16(hw_scale_output(y));

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::Clockwork)) {

            // Define bound for mu input buffers
            input_host.bound(x, 0, vec_width_fake).bound(y, 0, vec_height);
            input_gb.bound(x, 0, vec_width_fake).bound(y, 0, vec_height);
            input_cgra.bound_extent(x, vec_width_fake);

            scale_output.bound(y, 0, int(vec_height));
            hw_scale_output.bound(y, 0, int(vec_height));
            scale_output_glb.bound_extent(y, int(vec_height));
            scale_output_cgra.bound_extent(y, int(vec_height));

            // Define loop variables for scale_output output
            Var y_host, y_glb, y_cgra;
            Var x_host, x_glb, x_cgra;

            hw_scale_output.compute_root()
                .split(y, y_host, y_glb, int(vec_height))
                .reorder(y_glb, y_host)
                .hw_accelerate(y_glb, y_host);

            scale_output_glb.compute_at(hw_scale_output, y_host)
                .split(y, y_glb, y_cgra, int(vec_height))
                .reorder(y_cgra, y_glb);

            // Unroll output over glb (default 1)
            hw_scale_output.unroll(y_glb, glb_o);
            scale_output_glb.unroll(y_cgra, glb_o);

            // Compute each stage of the reduction tree inside tile_sum.
            for (int s = 1; s <= total_stages; s++) {
                tree_glb[s].compute_at(scale_output_glb, y_glb).unroll(x);
            }
            for (int s = 1; s <= total_stages; s++) {
                tree_fifo[s].compute_at(scale_output_glb, y_glb).unroll(x);
            }

            // Input buffers tiling schedule for mu input
            input_host.compute_root().accelerator_input();
            input_gb.compute_at(hw_scale_output, y_host);
            input_cgra.compute_at(scale_output_glb, y_glb);
            input_cgra
                .split(x, x_glb, x_cgra, tile_size)
                .reorder(x_cgra, y, x_glb);

            // Unroll inputs
            input_gb.unroll(x, glb_i);
            input_cgra.unroll(x_cgra, glb_i);

        } else {  // schedule to CPU
            scale_output_cgra.compute_root();
        }
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(Get_E8M0_Scale_Tree, get_e8m0_scale_tree_gb_input)

