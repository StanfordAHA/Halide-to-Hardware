#include "Halide.h"

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

class LayerNormPass2 : public Halide::Generator<LayerNormPass2> {
public:
    Input<Buffer<uint16_t>> input{ "input", 2 };
    Output<Buffer<uint16_t>> output{ "output", 2 };

    GeneratorParam<int> vec_width{ "vec_width", 768 };
    GeneratorParam<int> vec_height{ "vec_height", 128 };
    GeneratorParam<int> vec_width_fake{ "vec_width_fake", 2048 };
    GeneratorParam<int> vec_height_fake{ "vec_height_fake", 4 };
    GeneratorParam<int> glb_i{ "glb_i", 32 };
    GeneratorParam<int> tree_stages{ "tree_stages", 5 };

    void generate() {
        /* THE ALGORITHM */
        // Input: BF16 x - E(x) stored in 4 GLB Tiles
        // Output: BF16 out/(sqrt(sum(out^2)))*Nγ+β stored in 4 GLB Tiles
        const float gamma = 1.2f;
        const float beta = -0.35f;
        Var x("x"), y("y");
        Func hw_input("hw_input"), input_host("input_host"), input_glb("input_glb"), input_cgra("input_cgra");
        Func hw_output("hw_output"), output_glb("output_glb"), output_cgra("output_cgra"), sum_cgra("sum_cgra");

        hw_input(x, y) = bf16(input(x, y));
        input_host(x, y) = hw_input(x, y);
        input_glb(x, y) = input_host(x, y);
        input_cgra(x, y) = input_glb(x, y);

        const int tile_size = 1 << int(tree_stages);
        Expr num_tiles = vec_width_fake / tile_size;
        Var tile("tile"), xi("xi");
        Func tile_input("tile_input");
        // x = tile * tile_size + xi, 0 <= xi < tile_size
        tile_input(tile, xi, y) = input_cgra(tile * tile_size + xi, y) * input_cgra(tile * tile_size + xi, y);

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

        // After all stages, each tile produces one partial sum at xi = 0.
        Func tile_sum("tile_sum");
        // Here the “x” coordinate of tile_sum is interpreted as the tile index.
        tile_sum(tile, y) = tree[total_stages](0, tile, y);

        // --- Outer Reduction: Sum over all tiles ---
        RDom r(0, num_tiles, "r");
        sum_cgra(y) = bf16(0);
        sum_cgra(y) += tile_sum(r.x, y);

        // output_cgra(x, y) = input_cgra(x, y) / exp(bf16(0.5f) * log(sum_cgra(y))) * bf16(float(vec_width) * gamma) + bf16(beta);
        output_cgra(x, y) = bf16(sqrtf(float(vec_width)) * gamma) / exp(bf16(0.5f) * log(sum_cgra(y)));

        output_glb(x, y) = input_cgra(x, y) * output_cgra(x, y) + bf16(beta);
        hw_output(x, y) = output_glb(x, y);
        output(x, y) = u16(hw_output(x, y));

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::Clockwork)) {
            int mem_vec_width = tile_size;

            input_host.bound(x, 0, vec_width_fake).bound(y, 0, vec_height_fake);
            input_glb.bound(x, 0, vec_width_fake).bound(y, 0, vec_height_fake);
            input_cgra.bound_extent(x, mem_vec_width);

            output.bound(x, 0, vec_width_fake).bound(y, 0, vec_height_fake);
            hw_output.bound(x, 0, vec_width_fake).bound(y, 0, vec_height_fake);
            output_cgra.bound_extent(x, vec_width_fake);

            Var y_host, y_glb, y_cgra;
            Var x_host, x_glb, x_cgra;

            // Produce loop levels: host, global buffer, cgra
            // L3 loop level
            hw_output.compute_root();
            hw_output
                .split(x, x_host, x_glb, vec_width_fake)
                .split(y, y_host, y_glb, vec_height_fake)
                .reorder(x_glb, y_glb, x_host, y_host)
                .hw_accelerate(y_glb, y_host)
                .unroll(x_glb, glb_i);

            // L2 loop level
            output_glb.compute_at(hw_output, y_host);
            output_glb
                .split(x, x_glb, x_cgra, vec_width_fake)
                .split(y, y_glb, y_cgra, vec_height_fake)
                .reorder(x_cgra, y_cgra, x_glb, y_glb)
                .unroll(x_cgra, glb_i);

            // L1 loop level
            // Element-wise division
            output_cgra.compute_at(output_glb, y_glb);

            // Partial sum accumulator
            sum_cgra.compute_at(output_glb, y_glb);
            RVar rx_unroll("rx_unroll"), rx_cgra("rx_cgra"), rx_glb("rx_glb");
            Expr tile_unroll = 1;
            sum_cgra.update()
                .split(r.x, rx_glb, rx_cgra, num_tiles)
                .split(rx_cgra, rx_cgra, rx_unroll, tile_unroll)
                .reorder(rx_unroll, rx_cgra, y, rx_glb)
                .unroll(rx_unroll);

            // Reduction tree
            for (int s = 1; s <= total_stages; s++) {
                tree[s].compute_at(output_glb, y_glb).unroll(xi);
            }

            // Input streaming
            input_host.compute_root().accelerator_input();
            input_glb.compute_at(hw_output, y_host).unroll(x, glb_i);
            tile_input.compute_at(output_glb, y_glb)
                .unroll(xi, glb_i)
                .reorder(xi, tile, y);

        } else {  // schedule to CPU
            output_cgra.compute_root().unroll(x, glb_i);
        }
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(LayerNormPass2, layer_norm_pass2_fp)
