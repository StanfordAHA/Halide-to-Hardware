#include "Halide.h"

namespace {

using std::vector;
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

    // k_c determines the number of tiled channels
    GeneratorParam<int> k_ic{"k_ic", 4};    // default: 4
    GeneratorParam<int> k_oc{"k_oc", 4};    // default: 4
  
    // n_layers determines the total number of resnet layers
    // Change this using NLAYERS
    GeneratorParam<int> n_layers{"n_layers", 2};    // default: 2

    GeneratorParam<int> schedule{"schedule", 1};
  
    void generate() {
        //vector<int> pads =    { 1, 1, 1, 1 };
        vector<int> pads =    { 1, 0, 0, 0 };
        vector<int> ksizes =  { 3, 3, 3, 3 };
        vector<int> strides = { 1, 2, 1, 2 };
        // number of channels:
        //   number of  input channels for layer i at n_cs[i]
        //   number of output channels for layer i at n_cs[i+1]
        //vector<int> n_cs =    { 128, 256, 256, 512, 512 };
        vector<int> n_cs =    { 16, 32, 32, 64, 64 };
        
        int imgsize = 28;//FIXME//(in_img + 0 - ksize + 1) / stride;
        //Expr imgsize = u32(floor( (i16(in_img) + 2*i16(pad[0]) - i16(ksize[0])) / i16(stride[0])) + 1);
        int nlayers = n_layers;
        //kernel.resize(n_layers);

        vector<int> xysizes(nlayers);
        vector<int> xybounds(nlayers);
        for (int i=nlayers-1; i>=0; --i) {
          xysizes[i] =
            i==nlayers-1 ? 28 : // final output size
            xysizes[i+1] * strides[i+1] + 2*pads[i+1];
          xybounds[i] = 
            i==nlayers-1 ? 28 : // final output size
            i==nlayers-2 ? xybounds[i+1] * strides[i+1] :
            (xybounds[i+1] + 2*pads[i+2]) * strides[i+1];
        }

        /* THE ALGORITHM */
        Var x("x"), y("y"), z("z"), w("w"), zz("zz");

        Expr height = i16(imgsize);
        Expr width = i16(imgsize);

        Func hw_input("clamp_input");
        hw_input(z, x, y) = u16(input(z, clamp(x-pads[0], 0, width - 1), clamp(y-pads[0], 0, height - 1)));
        
        Func input_host("input_host");
        Func input_gb[nlayers];   fill_funcnames(input_gb, nlayers, "input_gb");
        Func input_cgra[nlayers]; fill_funcnames(input_cgra, nlayers, "input_cgra");
        input_host(z, x, y) = hw_input(z, x, y);

        Func hw_kernel[nlayers];   fill_funcnames(hw_kernel, nlayers, "hw_kernel");
        Func kernel_host[nlayers]; fill_funcnames(kernel_host, nlayers, "kernel_host");
        Func kernel_gb[nlayers];   fill_funcnames(kernel_gb, nlayers, "kernel_gb");
        Func kernel_cgra[nlayers]; fill_funcnames(kernel_cgra, nlayers, "kernel_cgra");

        for (int i=0; i<nlayers; ++i) {
          hw_kernel[i](z, w, x, y) = u16(kernel[i](z, w, x, y));
          kernel_host[i](z, w, x, y) = hw_kernel[i](z, w, x, y);
        }

        RDom r[nlayers];
        
        Func hw_output("hw_output");
        Func input_pad[nlayers];     fill_funcnames(input_pad, nlayers, "input_pad");
        Func output_host[nlayers];   fill_funcnames(output_host, nlayers, "output_host");
        Func output_gb[nlayers];     fill_funcnames(output_gb, nlayers, "output_gb");
        Func output_cgra[nlayers];   fill_funcnames(output_cgra, nlayers, "output_cgra");

        // Loop through all the layers
        for (int i=0; i<nlayers; ++i) {
          // Input image and kernel
          if (i == 0) {
            input_gb[i](z, x, y)         = input_host(z, x, y);
          } else {
            if (schedule == 1) {
              input_pad[i](z, x, y)        = output_host[i-1](clamp(x-pads[i], 0, xybounds[i] - 1), clamp(y-pads[i], 0, xybounds[i] - 1), z);
            } else {
              //input_pad[i](z, x, y)        = output_gb[i-1](clamp(x-pads[i], 0, xybounds[i] - 1), clamp(y-pads[i], 0, xybounds[i] - 1), z);
              input_pad[i](z, x, y)        = output_gb[i-1](x, y, z);
            }
            input_gb[i](z, x, y)         = input_pad[i](z, x, y);
          }
          input_cgra[i](zz, z, x, y)     = input_gb[i](k_ic*z + zz, x, y);

          kernel_gb[i](z, w, x, y)       = kernel_host[i](z, w, x, y);
          kernel_cgra[i](zz, z, w, x, y) = kernel_gb[i](k_ic*z + zz, w, x, y);

          // Convolution
          auto ic_outer = n_cs[i] / k_ic;
          r[i] = RDom(0, ksizes[i],
                      0, ksizes[i],
                      0, ic_outer,
                      0, k_ic);
          RDom R = r[i];

          output_cgra[i](x, y, w) +=
            kernel_cgra[i](R.w, R.z, w, R.x, R.y) *
            input_cgra[i](R.w, R.z, strides[i]*x + R.x, strides[i]*y + R.y);

          output_gb[i](x, y, w) = output_cgra[i](x, y, w);
          output_host[i](x, y, w) = output_gb[i](x, y, w);
        }

        hw_output(x, y, w) = output_host[nlayers-1](x, y, w); // use last output_host
        output(x, y, w) = max(0, u8(hw_output(x, y, w)));

        
        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR)) {

        } else if (get_target().has_feature(Target::Clockwork)) {
          // loop order: r.z, r.x, r.y, xi, yi, xo, yo
          Var x_host,y_host, x_gb,y_gb, x_cgra,y_cgra;
          Var xi,yi;
          Var w_cgra, w_gb;
          Var z_cgra, z_gb;
          RVar rz_cgra, rz_gb;

          if (schedule == 1) {
            // Schedule 1: each layer is done with a different accelerator call
            output.bound(x, 0, imgsize);
            output.bound(y, 0, imgsize);
            output.bound(w, 0, n_cs[nlayers]); // num output channels for the last layer
            //hw_output.bound(w, 0, n_cs[nlayers]);
            //hw_input.bound(z, 0, n_cs[0]);
            input_host.bound(z, 0, n_cs[0]);
          
            for (int i=0; i<nlayers; ++i) {
              output_cgra[i].bound(w, 0, n_cs[i+1]); // num output channels for each layer
              input_cgra[i].bound(zz, 0, k_ic);
              kernel_cgra[i].bound(zz, 0, k_ic);

              auto gbsize = imgsize;
              output_host[i].compute_root();
              //hw_output.unroll(w, k_oc);
              output_host[i]
                .tile(x, y, x_host,y_host, xi,yi, gbsize,gbsize)
                .reorder(xi,yi,w, x_host,y_host)
                .hw_accelerate(xi, x_host);

              auto tilesize = std::min(imgsize, 28);

              // Produce loop levels: host, global buffer, cgra
              output_gb[i].compute_at(output_host[i], x_host); // global buffer
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
              input_gb[i].compute_at(output_host[i], x_host); // global buffer
              //input_cgra.compute_at(output_gb, x_gb);   // mem tile
              input_cgra[i].compute_at(output_cgra[i], w_gb);   // mem tile

              // kernel buffers
              kernel_host[i].compute_root(); // host buffer
              kernel_host[i].accelerator_input();
              kernel_gb[i].compute_at(output_host[i], x_host); // global buffer
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

              // Compute the padding outside the accelerator
              //input_pad[i].compute_root();
            }
          
            input_host.compute_root(); // host buffer
            input_host.accelerator_input();
            
          } else if (schedule == 2) {
            // schedule 2: Create a single accelerator with a call for each layer
            output.bound(x, 0, imgsize);
            output.bound(y, 0, imgsize);
            output.bound(w, 0, n_cs[nlayers]); // num output channels for the last layer
            hw_output.bound(w, 0, n_cs[nlayers]);
            //hw_input.bound(z, 0, n_cs[0]);

            vector<int> xysizes(nlayers);
            for (int i=nlayers-1; i>=0; --i) {
              xysizes[i] =
                i==nlayers-1 ? 28 : // final output size
                xysizes[i+1] * strides[i+1] + 2*pads[i+1];
            }

          
            for (int i=0; i<nlayers; ++i) {
              output_cgra[i].bound(w, 0, n_cs[i+1]); // num output channels for each layer

              int ic_outer = 4;//FIXME//as<IntImm>(n_c[i] / k_ic).value;
              //hw_kernel.bound(z, 0, n_c[i]);

              //input_cgra[i].bound(z, 0, ic_outer);
              input_cgra[i].bound(zz, 0, k_ic);
              //kernel_cgra[i].bound(z, 0, ic_outer);
              kernel_cgra[i].bound(zz, 0, k_ic);
            }

            auto gbsize = imgsize;
            hw_output.compute_root();
            //hw_output.unroll(w, k_oc);
            hw_output
              .tile(x, y, x_host,y_host, xi,yi, gbsize,gbsize)
              .reorder(xi,yi,w, x_host,y_host)
              //.hw_accelerate(xi, x_host);
              .accelerator_output(x_host);

            for (int i=0; i<nlayers; ++i) {
              //auto tilesize = std::min(imgsize, 28);
              //auto tilesize = xysizes[i];
              auto tilesize = 56;

              // Produce loop levels: host, global buffer, cgra
              output_gb[i].compute_at(hw_output, x_host); // global buffer
              output_gb[i]
                .tile(x, y, x_gb,y_gb, x_cgra,y_cgra, tilesize,tilesize)
                .split(w, w_gb, w_cgra, k_oc)
                // reorder from inner to outermost
                .reorder(w_cgra, x_cgra, y_cgra,
                         x_gb, y_gb, w_gb);
              output_gb[i].accelerate_call_output(Var::outermost());

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
            
          } else {
            // default schedule: a single accelerator with a single call
            output.bound(x, 0, imgsize);
            output.bound(y, 0, imgsize);
            output.bound(w, 0, n_cs[nlayers]); // num output channels for the last layer
            hw_output.bound(w, 0, n_cs[nlayers]);
            //hw_input.bound(z, 0, n_cs[0]);

            vector<int> xysizes(nlayers);
            for (int i=nlayers-1; i>=0; --i) {
              xysizes[i] =
                i==nlayers-1 ? 28 : // final output size
                xysizes[i+1] * strides[i+1] + 2*pads[i+1];
            }

          
            for (int i=0; i<nlayers; ++i) {
              output_cgra[i].bound(w, 0, n_cs[i+1]); // num output channels for each layer

              int ic_outer = 4;//FIXME//as<IntImm>(n_c[i] / k_ic).value;
              //hw_kernel.bound(z, 0, n_c[i]);

              //input_cgra[i].bound(z, 0, ic_outer);
              input_cgra[i].bound(zz, 0, k_ic);
              //kernel_cgra[i].bound(z, 0, ic_outer);
              kernel_cgra[i].bound(zz, 0, k_ic);
            }

            auto gbsize = imgsize;
            hw_output.compute_root();
            //hw_output.unroll(w, k_oc);
            hw_output
              .tile(x, y, x_host,y_host, xi,yi, gbsize,gbsize)
              .reorder(xi,yi,w, x_host,y_host)
              .hw_accelerate(xi, x_host);

            for (int i=0; i<nlayers; ++i) {
              //auto tilesize = std::min(imgsize, 28);
              //auto tilesize = xysizes[i];
              auto tilesize = 56;

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

          }
          
        } else {  // schedule to CPU

        }
        
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(ResnetLayer, resnet_multi_layer)
