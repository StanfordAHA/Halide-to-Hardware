#include "Halide.h"

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

class ConvolutionKernel : public Halide::Generator<ConvolutionKernel> {
public:
    Input<Buffer<uint8_t>>  input{"input", 2};
    Output<Buffer<uint8_t>> output{"output", 2};

    void generate() {
        /* THE ALGORITHM */

        Var x("x"), y("y");

        Func kernel("kernel");
        Func conv("conv");
        //RDom r(0, 3,               0, 3);
        RDom r(0, 1,               0, 1);

        kernel(x,y) = 1;
        //kernel(0,0) = 11;      kernel(0,1) = 12;      kernel(0,2) = 13;
        //kernel(1,0) = 14;      kernel(1,1) = 0;       kernel(1,2) = 16;
        //kernel(2,0) = 17;      kernel(2,1) = 18;      kernel(2,2) = 19;

        //conv(x, y) = 0;

        Func hw_input("hw_input");
        Func input_host, input_cgra, input_copy;
        //hw_input(x, y) = u16(input(x, y));
        input_host(x, y) = u16(input(x, y)); // store this in the host
        input_cgra(x, y) = input_host(x, y); // store this in the cgra
        hw_input(x, y) = input_cgra(x, y); // store this in a mem tile
        
        //conv(x, y)  += u16(kernel(r.x, r.y)) * hw_input(x + r.x, y + r.y);
        conv(x, y)  += hw_input(x + r.x, y + r.y);
        //conv(x,y) =
        //  kernel(0,0)*hw_input(x,y) +
        //  kernel(1,0)*hw_input(x+1,y) +
        //  kernel(2,0)*hw_input(x+2,y) +
        //  kernel(0,1)*hw_input(x,y+1) +
        //  kernel(1,1)*hw_input(x+1,y+1) +
        //  kernel(2,1)*hw_input(x+2,y+1) +
        //  kernel(0,2)*hw_input(x,y+2) +
        //  kernel(1,2)*hw_input(x+1,y+2) +
        //  kernel(2,2)*hw_input(x+2,y+2);

        Func hw_output("hw_output"), output_cgra, output_host;
        output_cgra(x, y) = conv(x, y);
        //output_host(x, y) = output_cgra(x, y);
        hw_output(x, y) = output_cgra(x, y);
        //hw_output(x, y) = cast<uint8_t>(conv(x, y));

        output(x, y) = cast<uint8_t>(hw_output(x,y));

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR)) {

        } else if (get_target().has_feature(Target::Clockwork)) {
          Var x_host,y_host, x_cgra,y_cgra, x_pond,y_pond;
          Var xi,yi;

          int tilesize = 8;
          int cgrasize = tilesize * 4;

          hw_output.compute_root();
          //output.bound(x, 0, cgrasize*2); // 4 glb tiles
          //output.bound(y, 0, cgrasize*2);
          output.bound(x, 0, cgrasize);     // 1 glb tile
          output.bound(y, 0, cgrasize);
          
          // Produce loop levels: host, cgra, pond
          hw_output
            .tile(x, y, x_host,y_host, xi,yi, cgrasize,cgrasize)
            //.tile(xi, yi, x_cgra,y_cgra, x_pond,y_pond, tilesize,tilesize)
            //.reorder(x_pond, y_pond, x_cgra, y_cgra, x_host, y_host)
            .hw_accelerate(xi, x_host);

          //output_host.store_at(hw_output, x_host).compute_at(hw_output, x_host);
          
          output_cgra.compute_at(hw_output, x_host); // cgra memtile
          output_cgra.tile(x, y, x_cgra,y_cgra, x_pond,y_pond, tilesize,tilesize);

          // Unroll the computation loops to duplicate hardware
          bool do_unroll = false;
          if (do_unroll) {
            conv.update()
              .unroll(r.x)
              .unroll(r.y);
          }
          conv.compute_at(output_cgra, x_cgra); // memtile

          // compute the kernel values only once (they'll be converted to constants anyway)
          kernel.compute_at(output_cgra, x_cgra);

          // Three buffers: one at host,
          //                a copy stage as the cgra memtile,
          //                another copy stage as the memory tiles
          input_host.compute_root(); // host buffer
          input_host.accelerator_input();
          input_cgra.compute_at(hw_output, x_host); // cgra memtile
          hw_input.compute_at(output_cgra, x_cgra);   // mem tile

          //hw_input.compute_root();
          //hw_input.accelerator_input();
          //hw_input.in().compute_at(hw_output, x_host);
          //hw_input.in().in().compute_at(output_cgra, x_cgra);
          
        } else {  // schedule to CPU
/*          
          Var x_host,y_host, x_cgra,y_cgra, x_pond,y_pond;
          Var xi,yi;

          // Produce loop levels: host, cgra memtiles, pond
          output.tile(x, y, x_host,y_host, xi,yi, 256,256);
          output.tile(xi, yi, x_cgra,y_cgra, x_pond,y_pond, 64-2,64-2);

          // Three buffers: one at host,
          //                a copy stage as the cgra memtile,
          //                another copy stage as the memory tiles
          hw_input.compute_at(output_cgra, x_cgra);
          hw_input.store_root().compute_root();
          hw_input.in().store_at(output, x_host).compute_at(output,x_cgra);
//            .tag_as(cgra_memtile);
          hw_input.in().in().store_at(output, x_cgra).compute_at(output,x_pond);
//            .tag_as(host_dram);

          // Unroll the computation loops to duplicate hardware
          conv.update()
            .unroll(r.x)
            .unroll(r.y);

          // compute the kernel values only once (they'll be converted to constants anyway)
          kernel.compute_at(output, x_host);
*/
        }
        
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(ConvolutionKernel, three_level_pond)
