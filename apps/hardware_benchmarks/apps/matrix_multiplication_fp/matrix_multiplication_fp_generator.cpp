#include "Halide.h"

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

class MatrixMultiplication : public Halide::Generator<MatrixMultiplication> {
public:
    Input<Buffer<uint16_t>>  input{"input", 2};
    Input<Buffer<uint16_t>>  kernel{"kernel", 2};
    Output<Buffer<uint16_t>> output{"output", 2};

    GeneratorParam<int> input_height{"input_height", 64};
    GeneratorParam<int> input_height_tile{"input_height_tile", 64};

    GeneratorParam<int> input_width{"input_width", 8};
    GeneratorParam<int> input_width_unroll{"input_width_unroll", 8};

    GeneratorParam<int> kernel_width{"kernel_width", 8};
    GeneratorParam<int> kernel_width_unroll{"kernel_width_unroll", 8};

    // m_input_width determines the multiples of input channels in memory (m_input_width * input_width_unroll total)
    GeneratorParam<int> m_input_width{"m_input_width", 1};
  
    // m_kernel_width determines the multiples of output channels in memory (m_kernel_width * kernel_width_unroll total)
    GeneratorParam<int> m_kernel_width{"m_kernel_width", 1};

    // glb_i determines the input glb unrolling
    GeneratorParam<int> glb_i{"glb_i", 8};

    // glb_k determines the kernel glb unrolling
    GeneratorParam<int> glb_k{"glb_k", 8};

    // glb_o determines the output glb unrolling
    GeneratorParam<int> glb_o{"glb_o", 8};

    // Note, these are the halide gen args that I work:
    // input_height=64 input_height_tile=64 input_width=8 kernel_width=8 input_width_unroll=8 kernel_width_unroll=8 glb_i=8 glb_k=8 glb_o=8 m_input_width=1 m_kernel_width=1
    // input_height=128 input_height_tile=64 input_width=8 kernel_width=8 input_width_unroll=8 kernel_width_unroll=8 glb_i=8 glb_k=8 glb_o=8 m_input_width=1 m_kernel_width=1
    // input_height=256 input_height_tile=64 input_width=8 kernel_width=8 input_width_unroll=8 kernel_width_unroll=8 glb_i=8 glb_k=8 glb_o=8 m_input_width=1 m_kernel_width=1
    // input_height=1024 input_height_tile=64 input_width=8 kernel_width=8 input_width_unroll=8 kernel_width_unroll=8 glb_i=8 glb_k=8 glb_o=8 m_input_width=1 m_kernel_width=1
    // input_height=1024 input_height_tile=64 input_width=16 kernel_width=16 input_width_unroll=8 kernel_width_unroll=8 glb_i=8 glb_k=8 glb_o=8 m_input_width=1 m_kernel_width=1
    // input_height=1024 input_height_tile=64 input_width=32 kernel_width=32 input_width_unroll=8 kernel_width_unroll=8 glb_i=8 glb_k=8 glb_o=8 m_input_width=1 m_kernel_width=1
    // input_height=1024 input_height_tile=64 input_width=64 kernel_width=64 input_width_unroll=8 kernel_width_unroll=8 glb_i=8 glb_k=8 glb_o=8 m_input_width=1 m_kernel_width=1
  
    void generate() {
        assert((int)input_width >= (int)input_width_unroll * (int)m_input_width); // the number of total input channels must exceed unrolled channels
        assert((int)kernel_width >= (int)kernel_width_unroll * (int)m_kernel_width); // the number of total output channels must exceed unrolled channels
      
        /* THE ALGORITHM */

        Var x("x"), y("y"), w("w");

        int height = input_height;
        int width = 1;

        Func hw_input("hw_input"), hw_kernel("hw_kernel");
        Func input_host("input_host"), input_glb("input_glb"), input_cgra("input_cgra");        
        Func kernel_host("kernel_host"), kernel_glb("kernel_glb"), kernel_cgra("kernel_cgra");
        Func hw_output("hw_output"), output_glb("output_glb"), output_cgra("output_cgra");

        hw_input(x, y) = bf16(input(x, y));
        hw_kernel(w, x) = bf16(kernel(w, x));
      
        input_host(x, y) = hw_input(x, y);
        input_glb(x, y) = input_host(x, y);
        input_cgra(x, y) = input_glb(x, y);
      
        kernel_host(w, x) = hw_kernel(w, x);
        kernel_glb(w, x) = kernel_host(w, x);
        kernel_cgra(w, x) = kernel_glb(w, x);
      
        RDom r(0, input_width);

        output_cgra(w, y) += kernel_cgra(w, r.x) * input_cgra(r.x, y);
      
        output_glb(w, y) = output_cgra(w, y);
        hw_output(w, y) = output_glb(w, y);
        output(w, y) = u16(hw_output(w, y));
        
        /* THE SCHEDULE */
        if (get_target().has_feature(Target::Clockwork)) {
          
          // output.bound(x, 0, width);
          output.bound(y, 0, height);
          output.bound(w, 0, kernel_width);
          hw_output.bound(w, 0, kernel_width);

          hw_kernel.bound(x, 0, input_width);
          kernel_glb.bound(x, 0, input_width);
          input_host.bound(x, 0, input_width);

          int mem_kernel_width = (int)kernel_width_unroll * (int)m_kernel_width;
          int mem_input_width = (int)input_width_unroll * (int)m_input_width;
          
          output_cgra.bound_extent(w, mem_kernel_width);
          kernel_cgra.bound_extent(w, mem_kernel_width);
          input_cgra.bound_extent(x, mem_input_width);
          kernel_cgra.bound_extent(x, mem_input_width);
          
          kernel_glb.bound(w, 0, kernel_width);
          input_glb.bound(x, 0, input_width);
          input_cgra.bound_extent(x, input_width_unroll);
          kernel_cgra.bound_extent(x, input_width_unroll);
          kernel_cgra.bound_extent(w, kernel_width_unroll);
          
          int gbsize_y = height;
          int tilesize_y = input_height_tile;

          Var y_host, x_glb, y_glb, x_cgra, y_cgra;
          Var yi;
          Var w_cgra, w_glb;
          RVar rx_unroll("rx_unroll"), rx_cgra("rx_cgra"), rx_glb("rx_glb");

          // Produce loop levels: host, global buffer, cgra
          hw_output.compute_root();
          hw_output
            .split(y, y_host, yi, gbsize_y)
            .reorder(w, yi, y_host)
            .hw_accelerate(yi, y_host);

          output_glb.compute_at(hw_output, y_host); // global buffer
          output_glb
            .split(y, y_glb, y_cgra, tilesize_y)
            .split(w, w_glb, w_cgra, mem_kernel_width)
            .reorder(w_cgra, y_cgra,
                     w_glb, y_glb);

          // Unroll output over glb (default 1)
          hw_output.unroll(w, glb_o);
          output_glb.unroll(w_cgra, glb_o); 

          output_cgra.compute_at(output_glb, w_glb); // memtile
          output_cgra
            .reorder(w, y);

          output_cgra.update()
            .split(r.x, rx_glb, rx_cgra, mem_input_width)
            .split(rx_cgra, rx_cgra, rx_unroll, input_width_unroll)
            .reorder(rx_unroll, w, rx_cgra, y, rx_glb);

          output_cgra
            .unroll(w, kernel_width_unroll);
          output_cgra.update()
            .unroll(w, kernel_width_unroll)
            .unroll(rx_unroll, input_width_unroll); 
          
          // Three buffers: one at host,
          //                a copy stage as the global buffer,
          //                another copy stage as the memory tiles
          input_host.compute_root(); // host buffer
          input_host.accelerator_input();
          input_glb.compute_at(hw_output, y_host); // global buffer
          input_cgra.compute_at(output_cgra, rx_glb);   // mem tile

          // kernel buffers
          hw_kernel.compute_root();
          kernel_host.compute_root(); // host buffer
          kernel_host.accelerator_input();
          kernel_glb.compute_at(hw_output, y_host); // global buffer
          kernel_cgra.compute_at(output_cgra, rx_glb);   // mem tile

          input_cgra
            .split(x, x_glb, x_cgra, mem_input_width)
            .reorder(x_cgra, y, x_glb);
          kernel_cgra
            .split(x, x_glb, x_cgra, mem_input_width)
            .split(w, w_glb, w_cgra, mem_kernel_width)
            .reorder(w_cgra, x_cgra, w_glb, x_glb);

          // Unroll input and kernel over glb (default 1)
          kernel_glb.unroll(w, glb_k); // unroll glb input for small images
          kernel_cgra.unroll(w_cgra, glb_k); // unroll glb->cgra channels for small images
          input_glb.unroll(x, glb_i); // unroll glb input for small images
          input_cgra.unroll(x_cgra, glb_i); // unroll glb->cgra channels for small images

        } else {  // schedule to CPU
          output_cgra.compute_root();
        }
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(MatrixMultiplication, matrix_multiplication_fp)
