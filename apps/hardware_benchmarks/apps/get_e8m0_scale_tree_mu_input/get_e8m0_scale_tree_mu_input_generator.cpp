#include "Halide.h"

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

class Get_E8M0_Scale_Tree : public Halide::Generator<Get_E8M0_Scale_Tree> {
public:
    // mu_input: input from MU IOs
    // scale_output: unpacked e8m0 scale
    // unquantized_output: same as input, unquantized BF16
    Input<Buffer<uint16_t>> mu_input{ "mu_input", 2 };
    Output<Buffer<uint16_t>> scale_output{ "scale_output", 1 };

    // e.g. 56x56 image input, y dim
    GeneratorParam<int> vec_height{ "vec_height", 3136 };
    // e.g. 32 channels, x dim
    GeneratorParam<int> vec_width_fake{ "vec_width_fake", 32 };
    GeneratorParam<int> vec_width{ "vec_width", 64 };

    // mu_i determines the mu input unrolling
    GeneratorParam<int> mu_i{ "mu_i", 32 };

    // glb_o determines the output glb unrolling
    GeneratorParam<int> glb_o{ "glb_o", 2 };

    // tree_stages determines the number of stages in the reduction tree: log2(mu_i)
    GeneratorParam<int> tree_stages{ "tree_stages", 5 };

    void generate() {
        /* THE ALGORITHM */
        // Input: BF16 input from 32 MU IOs
        // Output 1: Unpacked e8m0 scale stored in 1 GLB Tile
        // Output 2: Same as input, unquantized BF16 stored in 32/8=4 GLB Tiles
        // Dimention variables
        Var x("x"), y("y");
        // MU input buffers
        Func mu_hw_input("mu_hw_input"), mu_input_host("mu_input_host"), mu_input_io("mu_input_io"), mu_input_cgra("mu_input_cgra");
        // scale_output output buffers
        Func hw_scale_output("hw_scale_output"), scale_output_glb("scale_output_glb"), scale_output_cgra("scale_output_cgra");

        // MU input buffers assignment
        mu_hw_input(x, y) = bf16(mu_input(x, y));
        mu_input_host(x, y) = mu_hw_input(x, y);
        mu_input_io(x, y) = mu_input_host(x, y);
        mu_input_cgra(x, y) = mu_input_io(x, y);

        // Define reduction trees
        const int tile_size = 1 << int(tree_stages);
        const int total_stages = int(tree_stages);
        std::vector<Func> tree_mu(total_stages + 1);
        std::vector<Func> tree_mem(total_stages + 1);
        tree_mu[0](x, y) = mu_input_cgra(x, y);
        tree_mem[0](x, y) = mu_input_cgra(x, y);

        // Reduction tree for mu input
        for (int s = 1; s <= total_stages; s++) {
            Func stage("tree_mu_" + std::to_string(s));
            // At stage s the effective domain (along xi) is ceil(tile_size / (2^s)).
            Expr prev_extent = (tile_size + ((1 << (s - 1)) - 1)) / (1 << (s - 1));
            stage(x, y) = abs_max_bf16(
                tree_mu[s - 1](2 * x, y),
                select((2 * x + 1) < prev_extent, tree_mu[s - 1](2 * x + 1, y), 0));
            tree_mu[s] = stage;
        }

        // Reduction tree for memory input
        for (int s = 1; s <= total_stages; s++) {
            Func stage("tree_mem_" + std::to_string(s));
            // At stage s the effective domain (along xi) is ceil(tile_size / (2^s)).
            Expr prev_extent = (tile_size + ((1 << (s - 1)) - 1)) / (1 << (s - 1));
            stage(x, y) = abs_max_bf16(
                tree_mem[s - 1](2 * x, y),
                select((2 * x + 1) < prev_extent, tree_mem[s - 1](2 * x + 1, y), 0));
            tree_mem[s] = stage;
        }

        // get e8m0 scale
        scale_output_cgra(y) = get_shared_exp(abs_max_bf16(tree_mu[total_stages](0, y), tree_mem[total_stages](0, y)));

        // data packing
        Func pack_out("pack_out");
        Var y_pack("y_pack");
        pack_out(y_pack) = bit8_pack(scale_output_cgra(2 * y_pack + 1), scale_output_cgra(2 * y_pack));

        // output buffers assignment
        // scale_output_glb(y_pack) = pack_out(y_pack);
        scale_output_glb(y_pack) = scale_output_cgra(y_pack);
        hw_scale_output(y_pack) = scale_output_glb(y_pack);
        scale_output(y_pack) = u16(hw_scale_output(y_pack));

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::Clockwork)) {

            // Define bound for mu input buffers
            mu_input_host.bound(x, 0, vec_width_fake).bound(y, 0, vec_height);
            mu_input_io.bound(x, 0, vec_width_fake).bound(y, 0, vec_height);
            mu_input_cgra.bound_extent(x, vec_width_fake);

            // Define bound for output buffers
            // scale_output.bound(y_pack, 0, int(vec_height/2));
            // hw_scale_output.bound(y_pack, 0, int(vec_height/2));
            // scale_output_glb.bound_extent(y_pack, int(vec_height/2));
            // scale_output_cgra.bound_extent(y, int(vec_height/2));
            // pack_out.bound_extent(y_pack, int(vec_height/2));

            scale_output.bound(y_pack, 0, int(vec_height));
            hw_scale_output.bound(y_pack, 0, int(vec_height));
            scale_output_glb.bound_extent(y_pack, int(vec_height));
            scale_output_cgra.bound_extent(y, int(vec_height));
            pack_out.bound_extent(y_pack, int(vec_height));

            // Define loop variables for scale_output output
            Var y_host, y_glb, y_cgra;
            Var x_host, x_glb, x_cgra;

            // Produce loop levels for scale_output output: host, glb, cgra
            // Host loop level
            // hw_scale_output.compute_root()
            //     .split(y_pack, y_host, y_glb, int(vec_height/2))
            //     .reorder(y_glb, y_host)
            //     .hw_accelerate(y_glb, y_host);

            hw_scale_output.compute_root()
                .split(y_pack, y_host, y_glb, int(vec_height))
                .reorder(y_glb, y_host)
                .hw_accelerate(y_glb, y_host);

            // GLB loop level
            // scale_output_glb.compute_at(hw_scale_output, y_host)
            //     .split(y_pack, y_glb, y_cgra, int(vec_height/2))
            //     .reorder(y_cgra, y_glb);

            scale_output_glb.compute_at(hw_scale_output, y_host)
                .split(y_pack, y_glb, y_cgra, int(vec_height))
                .reorder(y_cgra, y_glb);

            pack_out.compute_at(scale_output_glb, y_glb);

            // Unroll output over glb (default 1)
            hw_scale_output.unroll(y_glb, glb_o);
            scale_output_glb.unroll(y_cgra, glb_o);

            // Compute each stage of the reduction tree inside tile_sum.
            for (int s = 1; s <= total_stages; s++) {
                tree_mu[s].compute_at(scale_output_glb, y_glb).unroll(x);
            }
            for (int s = 1; s <= total_stages; s++) {
                tree_mem[s].compute_at(scale_output_glb, y_glb).unroll(x);
            }

            // Input buffers tiling schedule for mu input
            mu_input_host.compute_root().accelerator_input();
            mu_input_io.compute_at(hw_scale_output, y_host);
            mu_input_cgra.compute_at(scale_output_glb, y_glb);
            mu_input_cgra
                .split(x, x_glb, x_cgra, tile_size)
                .reorder(x_cgra, y, x_glb);

            // Unroll inputs
            mu_input_io.unroll(x, mu_i);
            mu_input_cgra.unroll(x_cgra, mu_i);

        } else {  // schedule to CPU
            scale_output_cgra.compute_root();
        }
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(Get_E8M0_Scale_Tree, get_e8m0_scale_tree_mu_input)

