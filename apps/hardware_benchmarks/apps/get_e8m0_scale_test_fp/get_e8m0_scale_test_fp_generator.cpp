#include "Halide.h"

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

class Get_E8M0_Scale : public Halide::Generator<Get_E8M0_Scale> {
public:
    Input<Buffer<uint16_t>> input{ "input", 2 };
    Input<Buffer<uint16_t>> input_2{ "input_2", 2 };
    Output<Buffer<uint16_t>> output{ "output", 1 };

    // 56x56 image input
    GeneratorParam<int> vec_height{ "vec_height", 3136 };
    // 32 channel
    GeneratorParam<int> vec_width{ "vec_width", 32 };

    // glb_i determines the input glb unrolling
    GeneratorParam<int> glb_i{ "glb_i", 32 };

    // glb_o determines the output glb unrolling
    GeneratorParam<int> glb_o{ "glb_o", 1 };

    // tree_stages determines the number of stages in the reduction tree: log2(vec_width)
    GeneratorParam<int> tree_stages{ "tree_stages", 5 };

    void generate() {
        /* THE ALGORITHM */
        Var x("x"), y("y");
        Func mu_hw_input("mu_hw_input"), hw_glb_input("hw_glb_input");
        Func mu_input_host("mu_input_host"), mu_input_glb("mu_input_glb"), mu_input_cgra("mu_input_cgra");
        Func glb_input_host("glb_input_host"), glb_input_glb("glb_input_glb"), glb_input_cgra("glb_input_cgra");
        Func hw_output("hw_output"), output_glb("output_glb"), output_cgra("output_cgra");

        mu_hw_input(x, y) = bf16(input(x, y));
        mu_input_host(x, y) = mu_hw_input(x, y);
        mu_input_glb(x, y) = mu_input_host(x, y);
        mu_input_cgra(x, y) = mu_input_glb(x, y);

        hw_glb_input(x, y) = bf16(input_2(x, y));
        glb_input_host(x, y) = hw_glb_input(x, y);
        glb_input_glb(x, y) = glb_input_host(x, y);
        glb_input_cgra(x, y) = glb_input_glb(x, y);

        const int tile_size = 1 << int(tree_stages);

        // Compute the number of stages needed
        const int total_stages = int(tree_stages);
        // Stage 0 simply passes along the input from the CGRA tile.
        std::vector<Func> tree_mu(total_stages + 1);
        std::vector<Func> tree_glb_in(total_stages + 1);
        tree_mu[0](x, y) = mu_input_cgra(x, y);
        tree_glb_in[0](x, y) = glb_input_cgra(x, y);

        // For each stage s, we reduce pairs of elements along xi.
        for (int s = 1; s <= total_stages; s++) {
            Func stage("tree_mu_" + std::to_string(s));
            // At stage s the effective domain (along xi) is ceil(tile_size / (2^s)).
            Expr prev_extent = (tile_size + ((1 << (s - 1)) - 1)) / (1 << (s - 1));
            stage(x, y) = abs_max_bf16(
                tree_mu[s - 1](2 * x, y),
                select((2 * x + 1) < prev_extent, tree_mu[s - 1](2 * x + 1, y), 0));
            tree_mu[s] = stage;
        }

        for (int s = 1; s <= total_stages; s++) {
            Func stage("tree_glb_in_" + std::to_string(s));
            // At stage s the effective domain (along xi) is ceil(tile_size / (2^s)).
            Expr prev_extent = (tile_size + ((1 << (s - 1)) - 1)) / (1 << (s - 1));
            stage(x, y) = abs_max_bf16(
                tree_glb_in[s - 1](2 * x, y),
                select((2 * x + 1) < prev_extent, tree_glb_in[s - 1](2 * x + 1, y), 0));
            tree_glb_in[s] = stage;
        }

        output_cgra(y) = get_shared_exp(abs_max_bf16(tree_mu[total_stages](0, y), tree_glb_in[total_stages](0, y)));

        Func pack_out("pack_out");
        Var y_pack("y_pack");
        pack_out(y_pack) = bit8_pack(output_cgra(2 * y_pack + 1), output_cgra(2 * y_pack));

        output_glb(y_pack) = pack_out(y_pack);
        hw_output(y_pack) = output_glb(y_pack);
        output(y_pack) = u16(hw_output(y_pack));

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::Clockwork)) {
            int mem_vec_width = tile_size;
            int packed_height = int(vec_height / 2);

            mu_input_host.bound(x, 0, int(vec_width))
                .bound(y, 0, vec_height);
            mu_input_glb.bound(x, 0, int(vec_width))
                .bound(y, 0, vec_height);
            mu_input_cgra.bound_extent(x, int(mem_vec_width));

            glb_input_host.bound(x, 0, int(vec_width))
                .bound(y, 0, vec_height);
            glb_input_glb.bound(x, 0, int(vec_width))
                .bound(y, 0, vec_height);
            glb_input_cgra.bound_extent(x, int(mem_vec_width));

            pack_out.bound_extent(y_pack, packed_height);
            output_cgra.bound_extent(y, vec_height);
            output.bound(y_pack, 0, packed_height);
            hw_output.bound(y_pack, 0, packed_height);
            output_glb.bound_extent( y_pack, packed_height);

            Var y_host, y_glb, y_cgra;
            Var x_host, x_glb, x_cgra;

            // Produce loop levels: host, global buffer, cgra
            // Host loop level
            hw_output.compute_root()
                .split(y_pack, y_host, y_glb, packed_height)
                .reorder(y_glb, y_host)
                .hw_accelerate(y_glb, y_host);

            // GLB loop level
            output_glb.compute_at(hw_output, y_host)
                .split(y_pack, y_glb, y_cgra, packed_height)
                .reorder(y_cgra, y_glb);

            pack_out.compute_at(output_glb, y_glb);

            // Unroll output over glb (default 1)
            hw_output.unroll(y_glb, glb_o);
            output_glb.unroll(y_cgra, glb_o);

            // Compute each stage of the reduction tree inside tile_sum.
            for (int s = 1; s <= total_stages; s++) {
                tree_mu[s].compute_at(output_glb, y_glb).unroll(x);
            }
            for (int s = 1; s <= total_stages; s++) {
                tree_glb_in[s].compute_at(output_glb, y_glb).unroll(x);
            }

            // Input buffers
            mu_input_host.compute_root().accelerator_input();
            mu_input_glb.compute_at(hw_output, y_host);
            mu_input_cgra.compute_at(output_glb, y_glb);
            mu_input_cgra
                .split(x, x_glb, x_cgra, tile_size)
                .reorder(x_cgra, y, x_glb);

            glb_input_host.compute_root().accelerator_input();
            glb_input_glb.compute_at(hw_output, y_host);
            glb_input_cgra.compute_at(output_glb, y_glb);
            glb_input_cgra
                .split(x, x_glb, x_cgra, tile_size)
                .reorder(x_cgra, y, x_glb);

            // Unroll input and kernel over glb (default 1)
            mu_input_glb.unroll(x, glb_i);  // unroll glb input for small images
            mu_input_cgra.unroll(x_cgra, glb_i); // unroll glb input for small images

            glb_input_glb.unroll(x, glb_i);  // unroll glb input for small images
            glb_input_cgra.unroll(x_cgra, glb_i); // unroll glb input for small images

        } else {  // schedule to CPU
            output_cgra.compute_root();
        }
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(Get_E8M0_Scale, get_e8m0_scale_test_fp)
