#include "Halide.h"

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

class ScalarSum : public Halide::Generator<ScalarSum> {
public:
    Input<Buffer<uint16_t>> input{ "input", 2 };
    Output<Buffer<uint16_t>> output{ "output", 1 };

    // After hacking, the scheduling actually does sum reduction across the entire vec_height x vec_width array
    GeneratorParam<int> vec_height{ "vec_height", 2 };
    GeneratorParam<int> vec_width{ "vec_width", 64 };

    // glb_i determines the input glb unrolling
    GeneratorParam<int> glb_i{ "glb_i", 1 };

    // glb_o determines the output glb unrolling
    GeneratorParam<int> glb_o{ "glb_o", 1 };

    // tree_stages determines the number of stages in the reduction tree
    GeneratorParam<int> tree_stages{ "tree_stages", 3 };

    void generate() {
        /* THE ALGORITHM */
        Var x("x"), y("y");
        Func hw_input("hw_input");
        Func input_host("input_host"), input_glb("input_glb"), input_cgra("input_cgra");
        Func hw_output("hw_output"), output_glb("output_glb"), output_cgra("output_cgra");

        hw_input(x, y) = bf16(input(x, y));
        input_host(x, y) = hw_input(x, y);
        input_glb(x, y) = input_host(x, y);
        input_cgra(x, y) = input_glb(x, y);

        const int tile_size = 1 << int(tree_stages);
        Expr num_tiles = vec_width / tile_size;
        // x = tile * tile_size + xi, 0 <= xi < tile_size
        Var tile("tile"), xi("xi");
        Func tile_input("tile_input");
        tile_input(tile, xi, y) = input_cgra(tile * tile_size + xi, y);

        // Compute the number of stages needed
        const int total_stages = int(tree_stages);
        // Stage 0 simply passes along the input from the CGRA tile.
        std::vector<Func> tree(total_stages + 1);
        tree[0](xi, tile, y) = tile_input(tile, xi, y);

        // For each stage s, we reduce pairs of elements along xi.
        for (int s = 1; s <= total_stages; s++) {
            Func stage("tree_" + std::to_string(s));
            // At stage s the effective domain (along xi) is ceil(tile_size / (2^s)).
            Expr prev_extent = (tile_size + ((1 << (s - 1)) - 1)) / (1 << (s - 1));
            stage(xi, tile, y) =
                tree[s - 1](2 * xi, tile, y) +
                select((2 * xi + 1) < prev_extent, tree[s - 1](2 * xi + 1, tile, y), 0);
            tree[s] = stage;
        }

        // After 5 stages, each tile produces one partial sum at xi = 0.
        Func tile_sum("tile_sum");
        // Here the “x” coordinate of tile_sum is interpreted as the tile index.
        tile_sum(tile, y) = tree[total_stages](0, tile, y);

        // --- Outer Reduction: Sum over all tiles ---
        RDom r(0, num_tiles, "r");
        output_cgra(y) = bf16(0);
        output_cgra(y) += tile_sum(r.x, y);

        output_glb(y) = output_cgra(y);
        hw_output(y) = output_glb(y);
        output(y) = u16(hw_output(y));

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::Clockwork)) {
            int mem_vec_width = tile_size;

            input_host.bound(x, 0, vec_width)
                .bound(y, 0, vec_height);
            input_glb.bound(x, 0, vec_width)
                .bound(y, 0, vec_height);
            input_cgra.bound_extent(x, mem_vec_width);

            output.bound(y, 0, vec_height);
            hw_output.bound(y, 0, vec_height);
            output_cgra.bound_extent(y, vec_height);

            Var y_host, y_glb, y_cgra;
            Var x_host, x_glb, x_cgra;

            // Produce loop levels: host, global buffer, cgra
            // Host loop level
            hw_output.compute_root();
            hw_output
                .split(y, y_host, y_glb, vec_height)
                .reorder(y_glb, y_host)
                .hw_accelerate(y_glb, y_host);

            // GLB loop level
            output_glb.compute_at(hw_output, y_host);  // global buffer
            output_glb
                .split(y, y_glb, y_cgra, vec_height)
                .reorder(y_cgra, y_glb);

            // Unroll output over glb (default 1)
            hw_output.unroll(y_glb, glb_o);
            output_glb.unroll(y_cgra, glb_o);

            output_cgra.compute_at(output_glb, y_glb);  // memtile
            RVar rx_unroll("rx_unroll"), rx_cgra("rx_cgra"), rx_glb("rx_glb");
            Expr tile_unroll = 1;
            output_cgra.update()
                .split(r.x, rx_glb, rx_cgra, num_tiles)
                .split(rx_cgra, rx_cgra, rx_unroll, tile_unroll)
                .reorder(rx_unroll, rx_cgra, y, rx_glb)
                .unroll(rx_unroll);

            // Compute each stage of the reduction tree inside tile_sum.
            for (int s = 1; s <= total_stages; s++) {
                tree[s].compute_at(output_glb, y_glb).unroll(xi);
            }

            // Input buffers
            input_host.compute_root().accelerator_input();
            input_glb.compute_at(hw_output, y_host);
            // input_cgra.compute_at(output_glb, y_glb);
            tile_input.compute_at(output_glb, y_glb);

            // input_cgra
            //     .split(x, x_glb, x_cgra, tile_size)
            //     .reorder(x_cgra, y, x_glb);
            tile_input.reorder(xi, tile, y);

            // Unroll input and kernel over glb (default 1)
            input_glb.unroll(x, glb_i);  // unroll glb input for small images
            // input_cgra.unroll(x_cgra, glb_i); // unroll glb input for small images
            tile_input.unroll(xi, glb_i);  // unroll glb input for small images

        } else {  // schedule to CPU
            output_cgra.compute_root();
        }
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(ScalarSum, scalar_reduction_fp)
