#include <iostream>
#include <math.h>
#include <cstdio>
#include <fstream>
#include <vector>
#include "hardware_process_helper.h"
#include "halide_image_io.h"

#if defined(WITH_CPU)
   #include "conv2D_fp.h"
#endif

#if defined(WITH_COREIR)
    #include "coreir_interpret.h"
#endif

#if defined(WITH_CLOCKWORK)
    #include "rdai_api.h"
    #include "clockwork_sim_platform.h"
    #include "conv2D_fp_clockwork.h"
#endif

using namespace Halide::Tools;
using namespace Halide::Runtime;

union {
  uint32_t val;
  float f;
} union_var;

uint16_t round_to_even_process(float a) {
  //uint32_t e = reinterpret_cast<uint32_t&>(a);
  union_var.f = a;
  uint32_t e = union_var.val;
  
  // round float to even, comment out this codeblock for truncation
  uint32_t half = 0x00008000;
  uint32_t sum = e + half;
  
  // check if bottom bits are all zero
  uint32_t mantissa_mask = 0x0000ffff;
  bool is_zeroed = (sum & mantissa_mask) == 0;
  
  // clear last bit (round even) on tie
  uint32_t clear_mask = ~( ((uint32_t)is_zeroed) << 16);
  e = sum & clear_mask;

  // clear bottom bits
  e = e >> 16;

  //return bfloat16_t::make_from_bits(float_to_bfloat16( expf(bfloat16_to_float(a.to_bits())) ));
  //return bfloat16_t::make_from_bits( (uint16_t)e );
  return (uint16_t)e;
}

// Similar routines for bfloat. It's somewhat simpler.
uint16_t float_to_bfloat16_process(float f) {
//    uint16_t ret[2];
//    memcpy(ret, &f, sizeof(float));
//    // Assume little-endian floats
//    return ret[1];
  return round_to_even_process(f);
}

float bfloat16_to_float_process(uint16_t b) {
    // Assume little-endian floats
    uint16_t bits[2] = {0, b};
    float ret;
    memcpy(&ret, bits, sizeof(float));
    return ret;
}

void saveHalideBufferToRawBigEndian(const Halide::Runtime::Buffer<uint16_t>& buffer, const std::string& filename) {
    std::ofstream out(filename, std::ios::binary);
    if (!out.is_open()) {
        std::cerr << "Failed to open file for writing: " << filename << std::endl;
        return;
    }

    // Iterate through each element of the buffer and write in big-endian order
    for (int i = 0; i < buffer.number_of_elements(); ++i) {
        uint16_t val = buffer(i);
        // Swap bytes if the system is little-endian
        uint16_t big_endian_val = (val << 8) | (val >> 8); // Swap bytes to convert to big-endian
        out.write(reinterpret_cast<const char*>(&big_endian_val), sizeof(big_endian_val));
    }

    out.close();
}

void loadRawDataToBuffer(const std::string& filename, Halide::Runtime::Buffer<uint16_t>& buffer) {
    std::ifstream inFile(filename, std::ios::binary);
    if (!inFile) {
        std::cerr << "Failed to open file for reading: " << filename << std::endl;
        return;
    }

    // Get the extents of each dimension in the buffer
    std::vector<int> extents(buffer.dimensions());
    for (int i = 0; i < buffer.dimensions(); i++) {
        extents[i] = buffer.dim(i).extent();
    }

    // Initialize indices to zero for all dimensions
    std::vector<int> indices(buffer.dimensions(), 0);

    // Function to recursively fill the buffer, with column-major order
    std::function<void(int)> fillBuffer = [&](int dim) {
        if (dim == -1) { // All dimensions are set
            uint16_t value;
            inFile.read(reinterpret_cast<char*>(&value), sizeof(value));
            if (!inFile) {
                std::cerr << "Error reading data for indices ";
                for (int i = 0; i < indices.size(); ++i) {
                    std::cerr << indices[i] << (i < indices.size() - 1 ? ", " : "");
                }
                std::cerr << std::endl;
                throw std::runtime_error("Failed to read data from file");
            }
            // Perform byte swap if necessary (for big-endian files)
            value = (value >> 8) | (value << 8);
            buffer(indices.data()) = value;
        } else { // Set the current dimension and recurse
            for (int i = 0; i < extents[dim]; ++i) {
                indices[dim] = i;
                fillBuffer(dim - 1);
            }
            indices[dim] = 0;
        }
    };

    try {
        // From the last dimension down to 0; reverse order for column-major)
        fillBuffer(buffer.dimensions() - 1);
    } catch (const std::exception& e) {
        std::cerr << "An exception occurred: " << e.what() << std::endl;
        inFile.close();
        return;
    }

    inFile.close();
}

int main( int argc, char **argv ) {
  std::map<std::string, std::function<void()>> functions;
  ManyInOneOut_ProcessController<uint16_t> processor("conv2D_fp", {"input", "kernel", "bias"});

  #if defined(WITH_CPU)
      auto cpu_process = [&]( auto &proc ) {
        conv2D_fp(proc.inputs["input"], proc.inputs["kernel"], proc.inputs["bias"], proc.output);
      };
      functions["cpu"] = [&](){ cpu_process( processor ); } ;
  #endif
  
  #if defined(WITH_COREIR)
      auto coreir_process = [&]( auto &proc ) {
          run_coreir_on_interpreter<>( "bin/design_top.json",
                                       proc.inputs["input_host_stencil.raw"], proc.output,
                                       "self.in_arg_0_0_0", "self.out_0_0" );
      };
      functions["coreir"] = [&](){ coreir_process( processor ); };
  #endif
  
  #if defined(WITH_CLOCKWORK)
      auto clockwork_process = [&]( auto &proc ) {
        RDAI_Platform *rdai_platform = RDAI_register_platform( &rdai_clockwork_sim_ops );
        if ( rdai_platform ) {
          printf( "[RUN_INFO] found an RDAI platform\n" );
          conv2D_fp_clockwork(proc.inputs["input"], proc.inputs["kernel"], proc.inputs["bias"], proc.output);
          RDAI_unregister_platform( rdai_platform );
        } else {
          printf("[RUN_INFO] failed to register RDAI platform!\n");
        }
      };
      functions["clockwork"] = [&](){ clockwork_process( processor ); };
  #endif

    // Add all defined functions
    processor.run_calls = functions;

    auto IX = getenv("in_img");
    auto PO = getenv("pad_o");
    auto K = getenv("ksize");
    auto S = getenv("stride");
    auto IC = getenv("n_ic");
    auto OC = getenv("n_oc");

    auto in_img = IX ? atoi(IX) : 161;
    auto pad_o = PO ? atoi(PO) : 1;
    auto ksize = K ? atoi(K) : 3;
    auto stride = S ? atoi(S) : 2;
    auto n_ic = IC ? atoi(IC) : 4;
    auto n_oc = OC ? atoi(OC) : 32;

    int X = in_img;
    int Y = X;
    int K_X = ksize;
    int K_Y = K_X;
    int PO_X = pad_o;
    int PO_Y = PO_X;
    int Z = n_ic; // input channel 
    int W = n_oc; // output channel

    int imgsize_x = std::floor( (X - K_X) / stride ) + 1 + 2 * pad_o;
    int imgsize_y = std::floor( (Y - K_Y) / stride ) + 1 + 2 * pad_o;

    processor.inputs_preset = true;
    std::cout << "using inputs set within process.cpp" << std::endl;
    
    ///// INPUT IMAGE /////
    processor.inputs["input"] = Halide::Runtime::Buffer<uint16_t>(Z, X, Y);
    auto input_copy = processor.inputs["input"];
    loadRawDataToBuffer("input_host_stencil.raw", input_copy);

    std::cout << "input has dims: " << processor.inputs["input"].dim(0).extent() << "x"
              << processor.inputs["input"].dim(1).extent() << "x"
              << processor.inputs["input"].dim(2).extent() << "\n";
    // print first element of input
    std::cout << "input(0,0,0) = " << input_copy(0,0,0) << "\n";

    ///// KERNEL WEIGHTS /////  
    processor.inputs["kernel"] = Halide::Runtime::Buffer<uint16_t>(W, Z, K_X, K_Y);
    auto kernel_copy = processor.inputs["kernel"];
    loadRawDataToBuffer("kernel_host_stencil.raw", kernel_copy);

    std::cout << "kernel has dims: " << processor.inputs["kernel"].dim(0).extent() << "x"
              << processor.inputs["kernel"].dim(1).extent() << "x"
              << processor.inputs["kernel"].dim(2).extent() << "x"
              << processor.inputs["kernel"].dim(3).extent() << "\n";
    // print first element of kernel
    std::cout << "kernel(0,0,0,0) = " << kernel_copy(0,0,0,0) << "\n";

    ///// BIAS /////
    processor.inputs["bias"] = Halide::Runtime::Buffer<uint16_t>(W);
    auto bias_copy = processor.inputs["bias"];
    loadRawDataToBuffer("bias_host_stencil.raw", bias_copy);

    std::cout << "bias has dims: " << processor.inputs["bias"].dim(0).extent() << "\n";
    // print first element of bias
    std::cout << "bias(0) = " << bias_copy(0) << "\n";

    ///// GOLD OUTPUTS /////
    processor.output = Halide::Runtime::Buffer<uint16_t>(W, imgsize_x, imgsize_y);

    // // Conv2D operation
    // for (int w = 0; w < W; w++) { // For each output channel
    //   for (int x = 0; x < imgsize_x; x++) {
    //     for (int y = 0; y < imgsize_y; y++) {
    //       float sum = 0.0f;
    //       for (int z = 0; z < Z; z++) { // For each input channel
    //         for (int kx = 0; kx < K_X; kx++) { // Kernel height
    //           for (int ky = 0; ky < K_Y; ky++) { // Kernel width
    //             int src_x = x * stride + kx;
    //             int src_y = y * stride + ky;
    //             if (src_x >= 0 && src_x < X && src_y >= 0 && src_y < Y) {
    //                 float val = bfloat16_to_float_process(input_copy(z, src_x, src_y));
    //                 float kernel_val = bfloat16_to_float_process(kernel_copy(w, z, kx, ky));
    //                 sum += val * kernel_val;
    //             }
    //           }
    //         }
    //       }
    //       sum += bfloat16_to_float_process(bias_copy(w));
    //       sum = std::min(std::max(sum, 0.0f), 6.0f);
    //       processor.output(w, x, y) = float_to_bfloat16_process(sum);
    //     }
    //   }
    // }

    std::cout << "output has dims: " << processor.output.dim(0).extent() << "x"
              << processor.output.dim(1).extent() << "x"
              << processor.output.dim(2).extent() << "\n";

    // std::cout << "Writing hw_output.mat to bin folder" << std::endl;
    // save_image(processor.output, "bin/hw_output.mat");

    // use provided inputs first: convert .mat to bin/.raw or copy .raw to bin/.raw
    return processor.process_command(argc, argv);
}  
