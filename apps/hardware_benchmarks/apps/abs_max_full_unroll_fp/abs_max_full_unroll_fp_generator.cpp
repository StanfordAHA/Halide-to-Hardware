#include "Halide.h"

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

class ScalarMax : public Halide::Generator<ScalarMax> {
public:
    Input<Buffer<uint16_t>> input{ "input", 2 };
    Output<Buffer<uint16_t>> output{ "output", 1 };

    GeneratorParam<int> vec_height{ "vec_height", 3136 };
    GeneratorParam<int> vec_width{ "vec_width", 32 };

    // glb_i determines the input glb unrolling
    GeneratorParam<int> glb_i{ "glb_i", 32 };

    // glb_o determines the output glb unrolling
    GeneratorParam<int> glb_o{ "glb_o", 1 };

    // tree_stages determines the number of stages in the reduction tree
    GeneratorParam<int> tree_stages{ "tree_stages", 5 };

    void generate() {
        /* THE ALGORITHM */
        Var x("x"), y("y");
        Func mu_hw_input("mu_hw_input");
        Func mu_input_host("mu_input_host"), mu_input_glb("mu_input_glb"), mu_input_cgra("mu_input_cgra");
        Func hw_output("hw_output"), output_glb("output_glb"), output_cgra("output_cgra");

        mu_hw_input(x, y) = bf16(input(x, y));
        mu_input_host(x, y) = mu_hw_input(x, y);
        mu_input_glb(x, y) = mu_input_host(x, y);
        mu_input_cgra(x, y) = mu_input_glb(x, y);

        const int tile_size = 1 << int(tree_stages);

        // Compute the number of stages needed
        const int total_stages = int(tree_stages);
        // Stage 0 simply passes along the input from the CGRA tile.
        std::vector<Func> tree(total_stages + 1);
        tree[0](x, y) = mu_input_cgra(x, y);

        // For each stage s, we reduce pairs of elements along xi.
        for (int s = 1; s <= total_stages; s++) {
            Func stage("tree_" + std::to_string(s));
            // At stage s the effective domain (along xi) is ceil(tile_size / (2^s)).
            Expr prev_extent = (tile_size + ((1 << (s - 1)) - 1)) / (1 << (s - 1));
            stage(x, y) = abs_max_bf16(
                tree[s - 1](2 * x, y),
                select((2 * x + 1) < prev_extent, tree[s - 1](2 * x + 1, y), 0));
            tree[s] = stage;
        }

        output_cgra(y) = tree[total_stages](0, y);

        output_glb(y) = output_cgra(y);
        hw_output(y) = output_glb(y);
        output(y) = u16(hw_output(y));

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::Clockwork)) {
            int mem_vec_width = tile_size;

            mu_input_host.bound(x, 0, vec_width)
                .bound(y, 0, vec_height);
            mu_input_glb.bound(x, 0, vec_width)
                .bound(y, 0, vec_height);
            mu_input_cgra.bound_extent(x, mem_vec_width);

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

            // Compute each stage of the reduction tree inside tile_sum.
            for (int s = 1; s <= total_stages; s++) {
                tree[s].compute_at(output_glb, y_glb).unroll(x);
            }

            // Input buffers
            mu_input_host.compute_root().accelerator_input();
            mu_input_glb.compute_at(hw_output, y_host);
            mu_input_cgra.compute_at(output_glb, y_glb);

            mu_input_cgra
                .split(x, x_glb, x_cgra, tile_size)
                .reorder(x_cgra, y, x_glb);

            // Unroll input and kernel over glb (default 1)
            mu_input_glb.unroll(x, glb_i);  // unroll glb input for small images
            mu_input_cgra.unroll(x_cgra, glb_i); // unroll glb input for small images

        } else {  // schedule to CPU
            output_cgra.compute_root();
        }
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(ScalarMax, abs_max_full_unroll_fp)
