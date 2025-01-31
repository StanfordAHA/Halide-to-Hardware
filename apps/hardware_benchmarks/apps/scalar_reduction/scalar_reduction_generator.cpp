#include "Halide.h"

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

class ScalarReduction : public Halide::Generator<ScalarReduction> {
public:
    Input<Buffer<int16_t>> input{ "input", 2 };
    Output<Buffer<int16_t>> output{ "output", 1 };

    GeneratorParam<int> num_vecs{ "num_vecs", 4 };
    GeneratorParam<int> num_vecs_tile{ "num_vecs_tile", 4 };

    GeneratorParam<int> vec_len{ "vec_len", 16 };
    GeneratorParam<int> vec_len_unroll{ "vec_len_unroll", 4 };

    // m_vec_len determines the multiples of vec slices in memory (m_vec_len * vec_len_unroll total)
    GeneratorParam<int> m_vec_len{ "m_vec_len", 4 };

    // glb_i determines the input glb unrolling
    GeneratorParam<int> glb_i{ "glb_i", 1 };

    // glb_o determines the output glb unrolling
    GeneratorParam<int> glb_o{ "glb_o", 1 };

    void generate() {
        assert((int) vec_len >= (int) vec_len_unroll * (int) m_vec_len);  // the total input vec length must exceed unrolling

        /* THE ALGORITHM */
        Var x("x"), y("y"), w("w");
        Func hw_input("hw_input");
        Func input_host("input_host"), input_glb("input_glb"), input_cgra("input_cgra");
        Func hw_output("hw_output"), output_glb("output_glb"), output_cgra("output_cgra");

        hw_input(x, y) = i16(input(x, y));
        input_host(x, y) = hw_input(x, y);
        input_glb(x, y) = input_host(x, y);
        input_cgra(x, y) = input_glb(x, y);

        RDom r(0, vec_len);
        output_cgra(y) += input_cgra(r.x, y);

        output_glb(y) = output_cgra(y);
        hw_output(y) = output_glb(y);
        output(y) = i16(hw_output(y));

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::Clockwork)) {
            int mem_vec_len = (int) vec_len_unroll * (int) m_vec_len;

            input_host.bound(x, 0, vec_len);
            input_host.bound(y, 0, num_vecs);
            input_glb.bound(x, 0, vec_len);
            input_glb.bound(y, 0, num_vecs);
            input_cgra.bound_extent(x, mem_vec_len);
            input_cgra.bound_extent(x, vec_len_unroll);

            output.bound(y, 0, num_vecs);
            hw_output.bound(y, 0, num_vecs);
            output_cgra.bound_extent(y, num_vecs_tile);

            int gbsize_y = num_vecs;
            int tilesize_y = num_vecs_tile;

            Var y_host, x_glb, y_glb, x_cgra, y_cgra;
            Var yi;
            RVar rx_unroll("rx_unroll"), rx_cgra("rx_cgra"), rx_glb("rx_glb");

            // Produce loop levels: host, global buffer, cgra
            hw_output.compute_root();
            hw_output
                .split(y, y_host, yi, gbsize_y)
                .reorder(yi, y_host)
                .hw_accelerate(yi, y_host);

            output_glb.compute_at(hw_output, y_host);  // global buffer
            output_glb
                .split(y, y_glb, y_cgra, tilesize_y);

            // Unroll output over glb (default 1)
            hw_output.unroll(yi, glb_o);
            output_glb.unroll(y_cgra, glb_o);

            output_cgra.compute_at(output_glb, y_glb);  // memtile

            output_cgra.update()
                .split(r.x, rx_glb, rx_cgra, mem_vec_len)
                .split(rx_cgra, rx_cgra, rx_unroll, vec_len_unroll)
                .reorder(rx_unroll, rx_cgra, y, rx_glb)
                .unroll(rx_unroll, vec_len_unroll);

            // Three buffers: one at host,
            //                a copy stage as the global buffer,
            //                another copy stage as the memory tiles
            input_host.compute_root();  // host buffer
            input_host.accelerator_input();
            input_glb.compute_at(hw_output, y_host);  // global buffer
            input_cgra.compute_at(output_cgra, rx_glb);  // mem tile

            input_cgra
                .split(x, x_glb, x_cgra, mem_vec_len)
                .reorder(x_cgra, y, x_glb);

            // Unroll input and kernel over glb (default 1)
            input_glb.unroll(x, glb_i);  // unroll glb input for small images
            input_cgra.unroll(x_cgra, glb_i);  // unroll glb->cgra channels for small images

        } else {  // schedule to CPU
            output_cgra.compute_root();
        }
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(ScalarReduction, scalar_reduction)
