#include "Halide.h"

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

class AvgPooling : public Halide::Generator<AvgPooling> {
public:
    Input<Buffer<int16_t>>  input{"input", 3};
    Input<Buffer<int16_t>>  filter_avg{"filter_avg", 3};
    Output<Buffer<int16_t>> output{"output", 3};

    // in_img determines the input image size
    GeneratorParam<int> in_img{"in_img", 7};    // default: 7
  
    // ksize determines the output stencil size
    GeneratorParam<uint8_t> ksize{"ksize", 7};    // default: 7
  
    // Stride determines the sampling rate for the down sample
    GeneratorParam<int>  stride{"stride", 7};  // default: 1

    // n_ic determines the number of input channels
    GeneratorParam<int> n_ic{"n_ic", 512};    // default: 8


    void generate() {
      
        /* THE ALGORITHM */
        int imgsize = floor( (int(in_img) - int(ksize)) / stride ) + 1;
        int tile_size = imgsize;
        Var x("x"), y("y"), c("c");
        Func input_host("input_host"), input_glb("input_glb"), input_cgra("input_cgra");
        Func filter_avg_host("filter_avg_host"), filter_avg_glb("filter_avg_glb"), filter_avg_cgra("filter_avg_cgra");
        Func avg_pooling("avg_pooling");
        Func hw_output("hw_output"), output_glb("output_glb"), output_cgra("output_cgra");

        input_host(c, x, y) = u16(input(c, x, y));
        input_glb(c, x, y) = input_host(c, x, y);
        input_cgra(c, x, y) = input_glb(c, x, y);

        filter_avg_host(c, x, y) = u16(filter_avg(c, x, y));
        filter_avg_glb(c, x, y) = filter_avg_host(c, x, y);
        filter_avg_cgra(c, x, y) = filter_avg_glb(c, x, y);

        RDom r(0, ksize, 0, ksize);
        avg_pooling(c, x, y) = u16(0);
        avg_pooling(c, x, y) += filter_avg_cgra(c, r.x, r.y) * input_cgra(c, stride * x + r.x, stride * y + r.y);

        output_cgra(c, x, y) = avg_pooling(c, x, y);
        output_glb(c, x, y) = output_cgra(c, x, y);
        hw_output(c, x, y) = output_glb(c, x, y);
        output(c, x, y) = i16(hw_output(c, x, y));

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::Clockwork)) {
            // Var c_outer, c_inner, x_outer, x_inner, y_outer, y_inner;
            // Var x_host,y_host, x_glb,y_glb, x_cgra,y_cgra;
            // Var xi,yi;
            // Var c_cgra, c_glb;

            Var xi, x_host, x_glb, x_cgra;
            Var yi, y_host, y_glb, y_cgra;
            Var c_glb, c_cgra;
    
            output.bound(x, 0, imgsize);
            output.bound(y, 0, imgsize);
            output.bound(c, 0, n_ic);

            hw_output.compute_root();
            hw_output
                .tile(x, y, x_host, y_host, xi, yi, imgsize, imgsize)
                .reorder(c, xi, yi, x_host, y_host)
                .hw_accelerate(xi, x_host);

            output_glb.compute_at(hw_output, x_host);
            output_glb
                .tile(x, y, x_glb, y_glb, x_cgra, y_cgra, tile_size, tile_size)
                .split(c, c_glb, c_cgra, 16)
                .reorder(c_cgra, x_cgra, y_cgra, c_glb, x_glb, y_glb);

            // hw_output.unroll(c, glb_o);
            // output_glb.unroll(c_cgra, glb_o);

            output_cgra.compute_at(output_glb, c_glb);
            // output_cgra.reorder(c, x, y);

            avg_pooling.compute_at(output_glb, c_glb);
            avg_pooling.unroll(c, 16);
            avg_pooling.update()
                .unroll(c, 16);

            input_cgra.compute_at(output_glb, c_glb);
            input_glb.compute_at(hw_output, x_host);
            input_host.compute_root();
            input_host.accelerator_input();

            filter_avg_cgra.compute_at(output_glb, c_glb);
            filter_avg_glb.compute_at(hw_output, x_host);
            filter_avg_host.compute_root();
            filter_avg_host.accelerator_input();
            
            // filter_avg_glb.unroll(c, 16);
            // filter_avg_cgra.unroll(c_cgra, 16);
            // input_glb.unroll(c, 16);
            // input_cgra.unroll(c_cgra, 16);
        }
    }
};
}  // namespace

HALIDE_REGISTER_GENERATOR(AvgPooling, avgpooling_cpu)
