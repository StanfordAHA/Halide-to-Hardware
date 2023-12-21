#include "Halide.h"

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

void fill_funcnames(Func funcs[], int len, std::string name) {
  for (int i=0; i<len; ++i) {
    funcs[i] = Func(name + "_" + std::to_string(i));
  }
}

class ResnetLayer : public Halide::Generator<ResnetLayer> {
public:
    Input<Buffer<uint8_t>>   input{"input", 3};
    Input<Buffer<uint8_t>[]> kernel{"kernel", 4};    
    Output<Buffer<uint8_t>>  output{"output", 3};

    // in_img determines the input image size
    GeneratorParam<int> in_img{"in_img", {28}};    // default: 28

    // pad determines the padding to the input image size
    Input<int[]> pad{"pad", 1};    // default: 1
  
    // ksize determines the output stencil size
    Input<int[]> ksize{"ksize", 3};    // default: 3
  
    // stride determines the sampling rate for the down sample
    Input<int[]>  stride{"stride", 1};  // default: 1

    // k_c determines the number of tiled channels
    GeneratorParam<int> k_ic{"k_ic", 4};    // default: 4
    GeneratorParam<int> k_oc{"k_oc", 4};    // default: 4
  
    // n_c determines the total number of channels
    Input<int[]> n_c{"n_c", 64};    // default: 64
  
      // n_layers determines the total number of resnet layers
    GeneratorParam<int> n_layers{"n_layers", 1};    // default: 4


    void generate() {
      int imgsize = 28;//FIXME//(in_img + 0 - ksize + 1) / stride;
      //Expr imgsize = u32(floor( (i16(in_img) + 2*i16(pad[0]) - i16(ksize[0])) / i16(stride[0])) + 1);
        int nlayers = n_layers;
        //kernel.resize(n_layers);
      
        /* THE ALGORITHM */
        Var x("x"), y("y"), z("z"), w("w"), zz("zz");

        Expr height = i16(imgsize);
        Expr width = i16(imgsize);

        Func hw_input("clamp_input");
        hw_input(z, x, y) = u16(input(z, clamp(x-pad[0], 0, width - 1), clamp(y-pad[0], 0, height - 1)));
        
        Func input_host("input_host");
        Func input_gb[nlayers];   fill_funcnames(input_gb, nlayers, "input_gb");
        Func input_cgra[nlayers]; fill_funcnames(input_cgra, nlayers, "input_cgra");
        input_host(z, x, y) = hw_input(z, x, y);

        Func hw_kernel[nlayers], kernel_host[nlayers], kernel_gb[nlayers], kernel_cgra[nlayers];
        for (int i=0; i<nlayers; ++i) {
          hw_kernel[i](z, w, x, y) = u16(kernel[i](z, w, x, y));
          kernel_host[i](z, w, x, y) = hw_kernel[i](z, w, x, y);
        }

        RDom r[nlayers];
        
        Func hw_output("hw_output");
        Func output_gb[nlayers];     fill_funcnames(output_gb, nlayers, "output_gb");
        Func output_cgra[nlayers];   fill_funcnames(output_cgra, nlayers, "output_cgra");

        // Loop through all the layers
        for (int i=0; i<nlayers; ++i) {
          // Input image and kernel
          if (i == 0) {
            input_gb[i](z, x, y)         = input_host(z, x, y);
          } else {
            input_gb[i](z, x, y)         = output_gb[i-1](z, x, y);
          }
          input_cgra[i](zz, z, x, y)     = input_gb[i](k_ic*z + zz, x, y);

          kernel_gb[i](z, w, x, y)       = kernel_host[i](z, w, x, y);
          kernel_cgra[i](zz, z, w, x, y) = kernel_gb[i](k_ic*z + zz, w, x, y);

          // Convolution
          auto ic_outer = n_c[i] / k_ic;
          r[i] = RDom(0, ksize[0],
                      0, ksize[0],
                      0, ic_outer,
                      0, k_ic);
          RDom R = r[i];

          output_cgra[i](x, y, w) +=
            kernel_cgra[i](R.w, R.z, w, R.x, R.y) *
            input_cgra[i](R.w, R.z, stride[i]*x + R.x, stride[i]*y + R.y);

          output_gb[i](x, y, w) = output_cgra[i](x, y, w);
        }

        hw_output(x, y, w) = output_gb[nlayers-1](x, y, w); // use last output_gb
        output(x, y, w) = max(0, u8(hw_output(x, y, w)));

        
        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR)) {

        } else if (get_target().has_feature(Target::Clockwork) || get_target().has_feature(Target::Pono)) {
          // loop order: r.z, r.x, r.y, xi, yi, xo, yo
          Var x_host,y_host, x_gb,y_gb, x_cgra,y_cgra;
          Var xi,yi;
          Var w_cgra, w_gb;
          Var z_cgra, z_gb;
          RVar rz_cgra, rz_gb;

          output.bound(x, 0, imgsize);
          output.bound(y, 0, imgsize);
          output.bound(w, 0, n_c[nlayers-1]);
          hw_output.bound(w, 0, n_c[nlayers-1]);

          auto gbsize = imgsize;
          hw_output.compute_root();
          //hw_output.unroll(w, k_oc);
          hw_output
            .tile(x, y, x_host,y_host, xi,yi, gbsize,gbsize)
            .reorder(xi,yi,w, x_host,y_host)
            .hw_accelerate(xi, x_host);

          for (int i=0; i<nlayers; ++i) {
            int ic_outer = 4;//FIXME//as<IntImm>(n_c[i] / k_ic).value;

            //hw_kernel.bound(z, 0, n_c[i]);
            //hw_input.bound(z, 0, n_c[i]);
            input_cgra[i].bound(z, 0, ic_outer);
            input_cgra[i].bound(zz, 0, k_ic);
            kernel_cgra[i].bound(z, 0, ic_outer);
            kernel_cgra[i].bound(zz, 0, k_ic);

            auto tilesize = std::min(imgsize, 28);

            // Produce loop levels: host, global buffer, cgra
            output_gb[i].compute_at(hw_output, x_host); // global buffer
            output_gb[i]
              .tile(x, y, x_gb,y_gb, x_cgra,y_cgra, tilesize,tilesize)
              .split(w, w_gb, w_cgra, k_oc)
              // reorder from inner to outermost
              .reorder(w_cgra, x_cgra, y_cgra,
                       x_gb, y_gb, w_gb);


            output_cgra[i].compute_at(output_gb[i], x_gb); // memtile
            output_cgra[i]
              .split(w, w_gb, w_cgra, k_oc)
              .reorder(w_cgra, x, y, w_gb);
            output_cgra[i].update()
              //.split(r.z, rz_gb, rz_cgra, k_ic)
              .split(w, w_gb, w_cgra, k_oc)
              .reorder(r[i].w, w_cgra, x, y, r[i].x, r[i].y, w_gb, r[i].z);

            Func output_rf;
            output_cgra[i].update()
              .unroll(w_cgra, k_oc)
              .unroll(r[i].w, k_ic); // this is the z reduction

            output_cgra[i].unroll(w_cgra, k_oc);

            // Three buffers: one at host,
            //                a copy stage as the global buffer,
            //                another copy stage as the memory tiles
            input_gb[i].compute_at(hw_output, x_host); // global buffer
            //input_cgra.compute_at(output_gb, x_gb);   // mem tile
            input_cgra[i].compute_at(output_cgra[i], w_gb);   // mem tile

            // kernel buffers
            kernel_host[i].compute_root(); // host buffer
            kernel_host[i].accelerator_input();
            kernel_gb[i].compute_at(hw_output, x_host); // global buffer
            //kernel_cgra.compute_at(output_gb, x_gb);   // mem tile
            kernel_cgra[i].compute_at(output_cgra[i], w_gb);   // mem tile

            //input_gb.unroll(z, k_ic);
            //input_cgra.unroll(z, k_ic);
            input_cgra[i]
              //.split(z, z_gb, z_cgra, k_ic)
              //.reorder(z_cgra, x, y, z_gb);
              .reorder(zz, x, y, z);
            kernel_cgra[i]
              //.split(z, z_gb, z_cgra, k_ic)
              //.reorder(z_cgra, w_cgra, x, y, z_gb, w_gb);
              .split(w, w_gb, w_cgra, k_oc)
              .reorder(zz, w_cgra, x, y, z, w_gb);
            
          }
          
          input_host.compute_root(); // host buffer
          input_host.accelerator_input();

          
        } else {  // schedule to CPU

        }
        
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(ResnetLayer, resnet_multi_layer)
