#include "Halide.h"

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

class ConvolutionKernel : public Halide::Generator<ConvolutionKernel> {
public:
    Input<Buffer<int16_t>>  input{"input", 3};
    Output<Buffer<int16_t>> output{"output", 3};

    void generate() {
        /* THE ALGORITHM */

      Var x("x"), y("y"), z("z"), k("k");
      Var k_pond("k_pond"), k_cgra("k_cgra"), x_pond("x_pond"), x_cgra("x_cgra");

        Func kernel("kernel");
        Func conv("conv");
        //     x     y     c
        RDom r(0, 1, 0, 1, 0, 8);

        kernel(x,y,z,k) = i16(1);
        //kernel(0,0) = 11;      kernel(0,1) = 12;      kernel(0,2) = 13;
        //kernel(1,0) = 14;      kernel(1,1) = 0;       kernel(1,2) = 16;
        //kernel(2,0) = 17;      kernel(2,1) = 18;      kernel(2,2) = 19;

        //conv(x, y) = 0;

        Func hw_input("hw_input");
        Func input_host, input_glb, input_cgra, input_pond;
        Func hw_output("hw_output"), output_pond, output_cgra, output_host;
        
        hw_input(z, x, y) = i16(input(z, x, y));
        //hw_input(z, x, y) = i16(input(z, clamp(x-pad, 0, in_img - 1), clamp(y-pad, 0, in_img - 1)));
        input_host(z, x, y) = hw_input(z, x, y);
        //input_glb(z, x, y) = input_host(z, x, y);
        input_cgra(z, x, y) = input_host(z, x, y);
        input_pond(z, x, y) = input_cgra(z, x, y);
        
        output_pond(k, x, y)  +=
          //kernel(r.x, r.y, r.z, k) *
          input_pond(r.z, x + r.x, y + r.y);

        //output_cgra(k, x, y) = output_pond(k, x, y);
        output_cgra(k_pond, x, y, k_cgra) = output_pond(k_pond + 4*k_cgra, x, y);
        //output_host(k, x, y) = output_cgra(k, x, y);
        hw_output(k_pond, k_cgra, x, y) = output_cgra(k_pond, x, y, k_cgra);
        //hw_output(k, x, y) = cast<uint8_t>(conv(k, x, y));

        output(k, x, y) = max(0, i16(hw_output(k%4, k/4, x,y)));

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR)) {

        } else if (get_target().has_feature(Target::Clockwork)) {
          Var x_host,y_host, x_cgra,y_cgra, x_pond,y_pond;
          Var k_cgra, k_pond;
          Var xi,yi;

          int tilesize = 8;
          int cgrasize = tilesize * 4;

          hw_output.compute_root();
          //output.bound(x, 0, cgrasize*2); // 4 glb tiles
          //output.bound(y, 0, cgrasize*2);
          output.bound(x, 0, 8);     // 1 glb tile
          output.bound(y, 0, 8);
          output.bound(k, 0, 8);
          input_host.bound(z, 0, 8);

          // ==== CGRA LEVEL ====
          // for k = 1:2
          //  for y = 1:8
          //   for x = 1:2
          // ==== POND LEVEL ====
          //       for x = 1:4
          //         for r.z = 1:8
          //          for k = 1:4
          //           output(k, x, y) += weights(r.z, k) * input(r.z, x, y) 
          
          // Produce loop levels: host, cgra, pond
          hw_output
            .tile(x, y, x_host,y_host, xi,yi, 8,8)
            //.tile(xi, yi, x_cgra,y_cgra, x_pond,y_pond, tilesize,tilesize)
            //.reorder(x_pond, y_pond, x_cgra, y_cgra, x_host, y_host)
            //.reorder(k,xi,yi, x_host,y_host)
            .reorder(k_pond,k_cgra,xi,yi, x_host,y_host)
            .hw_accelerate(xi, x_host);

          //output_host.store_at(hw_output, x_host).compute_at(hw_output, x_host);
          
          output_cgra.compute_at(hw_output, x_host); // cgra memtile
          output_cgra
            //.tile(x, k, x_cgra,k_cgra, x_pond,k_pond, 4,4)
            .split(x, x_cgra, x_pond, 4)
            .reorder(k_pond,x_pond, x_cgra,y,k_cgra);

          output_pond.compute_at(output_cgra, x_cgra);
          output_pond.update()
            .reorder(r.x, r.y, k, r.z, x, y);

          // Unroll the computation loops to duplicate hardware
          bool do_unroll = false;
          if (do_unroll) {
            output_pond.update()
              .unroll(r.x)
              .unroll(r.y);
          }

          // compute the kernel values only once (they'll be converted to constants anyway)
          //kernel.compute_at(output_cgra, x_cgra);

          // Three buffers: one at host,
          //                a copy stage as the cgra memtile,
          //                another copy stage as the memory tiles
          input_host.compute_root(); // host buffer
          input_host.accelerator_input();
          input_cgra.compute_at(hw_output, x_host); // cgra memtile
          input_pond.compute_at(output_cgra, x_cgra);   // mem tile

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
