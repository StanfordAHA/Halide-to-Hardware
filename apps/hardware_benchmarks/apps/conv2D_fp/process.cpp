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

void copyFile(const std::string &srcPath, const std::string &dstPath) {
    std::ifstream src(srcPath, std::ios::binary);
    std::ofstream dst(dstPath, std::ios::binary);

    if (!src.is_open() || !dst.is_open()) {
        throw std::runtime_error("Error opening files while copying from " + srcPath + " to " + dstPath);
    }

    dst << src.rdbuf();
}

bool file_exists(const std::string& name) {
    std::ifstream f(name.c_str());
    return f.good();
}

int main( int argc, char **argv ) {
  std::map<std::string, std::function<void()>> functions;
  ManyInOneOut_ProcessController<uint16_t> processor("conv2D_fp", {"input_host_stencil.mat", "kernel_host_stencil.mat", "bias_host_stencil.raw"});

  #if defined(WITH_CPU)
      auto cpu_process = [&]( auto &proc ) {
        conv2D_fp(proc.inputs["input_host_stencil.mat"], proc.inputs["kernel_host_stencil.mat"], proc.inputs["bias_host_stencil.raw"], proc.output);
      };
      functions["cpu"] = [&](){ cpu_process( processor ); } ;
  #endif
  
  #if defined(WITH_COREIR)
      auto coreir_process = [&]( auto &proc ) {
          run_coreir_on_interpreter<>( "bin/design_top.json",
                                       proc.inputs["input_host_stencil.mat"], proc.output,
                                       "self.in_arg_0_0_0", "self.out_0_0" );
      };
      functions["coreir"] = [&](){ coreir_process( processor ); };
  #endif
  
  #if defined(WITH_CLOCKWORK)
      auto clockwork_process = [&]( auto &proc ) {
        RDAI_Platform *rdai_platform = RDAI_register_platform( &rdai_clockwork_sim_ops );
        if ( rdai_platform ) {
          printf( "[RUN_INFO] found an RDAI platform\n" );
          conv2D_fp_clockwork(proc.inputs["input_host_stencil.mat"], proc.inputs["kernel_host_stencil.mat"], proc.inputs["bias_host_stencil.raw"], proc.output);
          RDAI_unregister_platform( rdai_platform );
        } else {
          printf("[RUN_INFO] failed to register RDAI platform!\n");
        }
      };
      functions["clockwork"] = [&](){ clockwork_process( processor ); };
  #endif

    // Add all defined functions
    processor.run_calls = functions;

    auto OX = getenv("in_img");
    auto K = getenv("ksize");
    auto S = getenv("stride");
    auto IC = getenv("n_ic");
    auto OC = getenv("n_oc");
    auto RELU6 = getenv("relu6");
    auto PO_L = getenv("pad_o_left");
    auto PO_R = getenv("pad_o_right");
    auto use_torch_gold = getenv("TORCH_GOLD_LAYER");

    auto in_img = OX ? atoi(OX) : 56;
    auto ksize = K ? atoi(K) : 3;
    auto stride = S ? atoi(S) : 1;
    auto n_ic = IC ? atoi(IC) : 16;
    auto n_oc = OC ? atoi(OC) : 8;
    auto relu6 = RELU6 ? atoi(RELU6) : 1;
    auto pad_o_left = PO_L ? atoi(PO_L) : 0;
    auto pad_o_right = PO_R ? atoi(PO_R) : 0;
    std::string use_torch_gold_str = use_torch_gold ? use_torch_gold : "";

    int X = in_img;
    int Y = X;
    int K_X = ksize;
    int K_Y = K_X;
    int Z = n_ic; // input channel 
    int W = n_oc; // output channel
    int use_relu6 = relu6;
    int PO_LEFT = pad_o_left;
    int PO_RIGHT = pad_o_right;

    if (true) {//OX || P || K || S || IC || OC) {
      std::cout << "using inputs set within process.cpp" << std::endl;
      processor.inputs_preset = true;
    } else {
      std::cout << "reading input_host_stencil.mat and kernel_host_stencil.mat" << std::endl;
      processor.inputs_preset = false;
    }
    
    ///// INPUT IMAGE /////
    processor.inputs["input_host_stencil.mat"] = Halide::Runtime::Buffer<uint16_t>(Z, X, Y);
    auto input_copy_stencil = processor.inputs["input_host_stencil.mat"];
    int in_sparsity = 0;
    for (int y = 0; y < input_copy_stencil.dim(2).extent(); y++) {
      for (int x = 0; x < input_copy_stencil.dim(1).extent(); x++) {
        for (int z = 0; z < input_copy_stencil.dim(0).extent(); z++) {
          if (rand() % 100 < in_sparsity) {
            input_copy_stencil(z, x, y) = float_to_bfloat16_process(0.0f);
          } else {
            input_copy_stencil(z, x, y) = float_to_bfloat16_process(
              // [-0.05, 0.05]
              ((float)rand() / RAND_MAX) * 0.1 - 0.05
            );
          }
          // std::cout << "input: " << " z: " << z << " x: " << x << " y: " << y << " val: " << bfloat16_to_float_process(input_copy_stencil(z, x, y)) << std::endl;
        }
      }
    }

    std::cout << "input has dims: " << processor.inputs["input_host_stencil.mat"].dim(0).extent() << "x"
              << processor.inputs["input_host_stencil.mat"].dim(1).extent() << "x"
              << processor.inputs["input_host_stencil.mat"].dim(2).extent() << "\n";

    ///// KERNEL WEIGHTS /////  
    processor.inputs["kernel_host_stencil.mat"] = Halide::Runtime::Buffer<uint16_t>(W, Z, K_X, K_Y);
    auto kernel_copy_stencil = processor.inputs["kernel_host_stencil.mat"];
    int kernel_sparsity = 0;
    for (int y = 0; y < kernel_copy_stencil.dim(3).extent(); y++) {
      for (int x = 0; x < kernel_copy_stencil.dim(2).extent(); x++) {
        for (int z = 0; z < kernel_copy_stencil.dim(1).extent(); z++) {
          for (int w = 0; w < kernel_copy_stencil.dim(0).extent(); w++) {
            if (rand() % 100 < kernel_sparsity) {
              kernel_copy_stencil(w, z, x, y) = float_to_bfloat16_process(0.0f);
            } else {
              kernel_copy_stencil(w, z, x, y) = float_to_bfloat16_process(
                // [0, 0.005]
                ((float)rand() / RAND_MAX) * 0.005
              );
            }
            // std::cout << "kernel: " << " w: " << w << " z: " << z << " x: " << x << " y: " << y << " val: " << bfloat16_to_float_process(kernel_copy_stencil(w, z, x, y)) << std::endl;
          } 
        } 
      } 
    }

    std::cout << "kernel has dims: " << processor.inputs["kernel_host_stencil.mat"].dim(0).extent() << "x"
              << processor.inputs["kernel_host_stencil.mat"].dim(1).extent() << "x"
              << processor.inputs["kernel_host_stencil.mat"].dim(2).extent() << "x"
              << processor.inputs["kernel_host_stencil.mat"].dim(3).extent() << "\n";

    ///// BIAS /////
    processor.inputs["bias_host_stencil.raw"] = Halide::Runtime::Buffer<uint16_t>(W);
    auto bias_copy_stencil = processor.inputs["bias_host_stencil.raw"];
    for (int w = 0; w < bias_copy_stencil.dim(0).extent(); w++) {
      bias_copy_stencil(w) = float_to_bfloat16_process(
        // [-0.05, 0.05]
        ((float)rand() / RAND_MAX) * 0.1 - 0.05
      );
    }

    std::cout << "bias has dims: " << processor.inputs["bias_host_stencil.raw"].dim(0).extent() << "\n";

    ///// GOLD OUTPUTS /////
    int imgsize_x = std::floor( (X - K_X) / stride ) + 1;
    int imgsize_y = std::floor( (Y - K_Y) / stride ) + 1;
    int imgsize_x_padded = imgsize_x + PO_LEFT + PO_RIGHT;
    int imgsize_y_padded = imgsize_y + PO_LEFT + PO_RIGHT;
    processor.output = Halide::Runtime::Buffer<uint16_t>(W, imgsize_x, imgsize_y);
    auto output_gold_tensor = Halide::Runtime::Buffer<uint16_t>(W, imgsize_x_padded, imgsize_y_padded);
    output_gold_tensor.fill(0);

    // Conv2D operation
    for (int w = 0; w < W; w++) { // For each output channel
      for (int x = 0; x < imgsize_x; x++) {
        for (int y = 0; y < imgsize_y; y++) {
          float sum = 0.0f;
          for (int z = 0; z < Z; z++) { // For each input channel
            for (int kx = 0; kx < K_X; kx++) { // Kernel height
              for (int ky = 0; ky < K_Y; ky++) { // Kernel width
                int src_x = x * stride + kx;
                int src_y = y * stride + ky;
                if (src_x >= 0 && src_x < X && src_y >= 0 && src_y < Y) {
                    float val = bfloat16_to_float_process(input_copy_stencil(z, src_x, src_y));
                    float kernel_val = bfloat16_to_float_process(kernel_copy_stencil(w, z, kx, ky));
                    sum += val * kernel_val;
                }
              }
            }
          }
          sum += bfloat16_to_float_process(bias_copy_stencil(w));
          if (use_relu6) sum = std::min(std::max(sum, 0.0f), 6.0f);
          output_gold_tensor(w, x + PO_LEFT, y + PO_LEFT) = float_to_bfloat16_process(sum);
        }
      }
    }

    std::cout << "output has dims: " << output_gold_tensor.dim(0).extent() << "x"
              << output_gold_tensor.dim(1).extent() << "x"
              << output_gold_tensor.dim(2).extent() << "\n";

    // use provided inputs first: convert .mat to bin/.raw or copy .raw to bin/.raw
    if (use_torch_gold_str == "") {
      if (file_exists("input_host_stencil.mat")) {
        std::cout << "Removing existing input_host_stencil.mat" << std::endl;
        remove("input_host_stencil.mat");
      }
      std::cout << "Writing input_host_stencil.mat to bin folder" << std::endl;
      save_image(processor.inputs["input_host_stencil.mat"], "bin/input_host_stencil.mat");

      if (file_exists("kernel_host_stencil.mat")) {
        std::cout << "Removing existing kernel_host_stencil.mat" << std::endl;
        remove("kernel_host_stencil.mat");
      }
      std::cout << "Writing kernel_host_stencil.mat to bin folder" << std::endl;
      save_image(processor.inputs["kernel_host_stencil.mat"], "bin/kernel_host_stencil.mat");

      if (file_exists("bias_host_stencil.raw")) {
        std::cout << "Removing existing bias_host_stencil.raw" << std::endl;
        remove("bias_host_stencil.raw");
      }
      std::cout << "Writing bias_host_stencil.raw to bin folder" << std::endl;
      saveHalideBufferToRawBigEndian(processor.inputs["bias_host_stencil.raw"], "bin/bias_host_stencil.raw");

      if (file_exists("hw_output.mat")) {
        std::cout << "Removing existing hw_output.mat" << std::endl;
        remove("hw_output.mat");
      }
      std::cout << "Writing hw_output.mat to bin folder" << std::endl;
      save_image(output_gold_tensor, "bin/hw_output.mat");
    } else {
      std::cout << "Reading input_host_stencil.mat from " << "pytorch_gold/" << use_torch_gold << std::endl;
      copyFile("pytorch_gold/" + use_torch_gold_str + "/input_host_stencil.mat", "./input_host_stencil.mat");

      std::cout << "Reading kernel_host_stencil.mat from " << "pytorch_gold/" << use_torch_gold << std::endl;
      copyFile("pytorch_gold/" + use_torch_gold_str + "/kernel_host_stencil.mat", "./kernel_host_stencil.mat");

      // Have to use raw for bias since Halide mat2raw has issues with 1D array
      std::cout << "Reading bias_host_stencil.raw from " << "pytorch_gold/" << use_torch_gold << std::endl;
      copyFile("pytorch_gold/" + use_torch_gold_str + "/bias_host_stencil.raw", "./bias_host_stencil.raw");

      std::cout << "Reading hw_output.mat from " << "pytorch_gold/" << use_torch_gold << std::endl;
      copyFile("pytorch_gold/" + use_torch_gold_str + "/hw_output.mat", "./hw_output.mat");
    }

    return processor.process_command(argc, argv);
}  
