#include "Halide.h"

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

class FullyConnectedLayer : public Halide::Generator<FullyConnectedLayer> {
public:
    Input<Buffer<uint16_t>> matrix{ "matrix", 2 };
    Input<Buffer<uint16_t>> vector{ "vector", 1 };
    Output<Buffer<uint16_t>> output{ "output", 1 };

    // matrix_width is the reduction dimension
    // vector size is (matrix_width,)
    GeneratorParam<int> matrix_width{ "matrix_width", 32 };
    GeneratorParam<int> matrix_height{ "matrix_height", 64 };
    GeneratorParam<int> matrix_width_fake{ "matrix_width_fake", 64 };
    GeneratorParam<int> matrix_height_fake{ "matrix_height_fake", 4 };

    // glb_i determines the matrix and vector glb unrolling
    GeneratorParam<int> glb_i{ "glb_i", 8 };

    // glb_o determines the output glb unrolling
    GeneratorParam<int> glb_o{ "glb_o", 1 };

    // tree_stages determines the number of stages in the reduction tree
    GeneratorParam<int> tree_stages{ "tree_stages", 3 };

    void generate() {
        /* THE ALGORITHM */
        Var x("x"), y("y");
        Func hw_matrix("hw_matrix"), hw_vector("hw_vector");
        Func matrix_host("matrix_host"), matrix_glb("matrix_glb"), matrix_cgra("matrix_cgra");
        Func vector_host("vector_host"), vector_glb("vector_glb"), vector_cgra("vector_cgra");
        Func hw_output("hw_output"), output_glb("output_glb"), output_cgra("output_cgra");

        hw_matrix(x, y) = bf16(matrix(x, y));
        matrix_host(x, y) = hw_matrix(x, y);
        matrix_glb(x, y) = matrix_host(x, y);
        matrix_cgra(x, y) = matrix_glb(x, y);

        hw_vector(x) = bf16(vector(x));
        vector_host(x) = hw_vector(x);
        vector_glb(x) = vector_host(x);
        vector_cgra(x) = vector_glb(x);

        const int tile_size = 1 << int(tree_stages);
        Expr num_tiles = matrix_width_fake / tile_size;
        // x = tile * tile_size + xi, 0 <= xi < tile_size
        Var tile("tile"), xi("xi");
        Func tile_input("tile_input");
        // tile_input(tile, xi, y) = matrix_cgra(tile * tile_size + xi, y);
        tile_input(tile, xi, y) = matrix_cgra(tile * tile_size + xi, y) * bf16(2.0f);

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

            matrix_host.bound(x, 0, matrix_width_fake)
                .bound(y, 0, matrix_height_fake);
            matrix_glb.bound(x, 0, matrix_width_fake)
                .bound(y, 0, matrix_height_fake);
            matrix_cgra.bound_extent(x, mem_vec_width);

            vector_host.bound(x, 0, matrix_width_fake);
            vector_glb.bound(x, 0, matrix_width_fake);
            vector_cgra.bound_extent(x, mem_vec_width);

            output.bound(y, 0, matrix_height_fake);
            hw_output.bound(y, 0, matrix_height_fake);
            output_cgra.bound_extent(y, matrix_height_fake);

            Var y_host, y_glb, y_cgra;
            Var x_host, x_glb, x_cgra;

            // Produce loop levels: host, global buffer, cgra
            // Host loop level
            hw_output.compute_root();
            hw_output
                .split(y, y_host, y_glb, matrix_height_fake)
                .reorder(y_glb, y_host)
                .hw_accelerate(y_glb, y_host);

            // GLB loop level
            output_glb.compute_at(hw_output, y_host);  // global buffer
            output_glb
                .split(y, y_glb, y_cgra, matrix_height_fake)
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
            matrix_host.compute_root().accelerator_input();
            matrix_glb.compute_at(hw_output, y_host);
            vector_host.compute_root().accelerator_input();
            vector_glb.compute_at(hw_output, y_host);

            tile_input.compute_at(output_glb, y_glb);

            matrix_cgra
                .split(x, x_glb, x_cgra, tile_size)
                .reorder(x_cgra, y, x_glb);
            // vector_cgra
            //     .split(x, x_glb, x_cgra, tile_size)
            //     .reorder(x_cgra, x_glb)
            //     .compute_at(output_glb, y_glb);
            tile_input.reorder(xi, tile, y);

            // Unroll input and kernel over glb (default 1)
            matrix_glb.unroll(x, glb_i);  // unroll glb input for small images
            vector_glb.unroll(x, glb_i);  // unroll glb input for small images
            tile_input.unroll(xi, glb_i);  // unroll glb input for small images

        } else {  // schedule to CPU
            output_cgra.compute_root();
        }
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(FullyConnectedLayer, fully_connected_layer_fp)
