#include <iostream>
#include <math.h>
#include <cstdio>
#include <fstream>
#include <vector>
#include "hardware_process_helper.h"
#include "halide_image_io.h"
#include "coreir.h"

#if defined(WITH_CPU)
   #include "depthwise_conv_preload_fp.h"
#endif

#if defined(WITH_COREIR)
    #include "coreir_interpret.h"
#endif

#if defined(WITH_CLOCKWORK)
    #include "rdai_api.h"
    #include "clockwork_sim_platform.h"
    #include "depthwise_conv_preload_fp_clockwork.h"
#endif

using namespace Halide::Tools;
using namespace Halide::Runtime;

nlohmann::json output_starting_json;

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

std::vector<int> parse_glb_bank_config_env_var(const std::string& env_var_name) {
    std::vector<int> values;
    const char* env_var_value = std::getenv(env_var_name.c_str());

    if (env_var_value) {
        std::string value_str = env_var_value;
        std::istringstream iss(value_str);
        std::string token;

        // Split the string by commas and convert to integers
        while (std::getline(iss, token, ',')) {
            // Trim potential whitespace
            token.erase(0, token.find_first_not_of(" \t\n\r\f\v"));
            token.erase(token.find_last_not_of(" \t\n\r\f\v") + 1);
            values.push_back(std::stoi(token));
        }
    } else {
        std::cerr << "Environment variable " << env_var_name << " not found." << std::endl;
    }

    return values;
}

int main( int argc, char **argv ) {
  std::map<std::string, std::function<void()>> functions;
  ManyInOneOut_ProcessController<uint16_t> processor("depthwise_conv_preload_fp", {"input_host_stencil.mat"});

  #if defined(WITH_CPU)
      auto cpu_process = [&]( auto &proc ) {
        depthwise_conv_preload_fp(proc.inputs["input_host_stencil.mat"], proc.output);
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
          depthwise_conv_preload_fp_clockwork(proc.inputs["input_host_stencil.mat"], proc.output);
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
    auto KX = getenv("ksize");
    auto S = getenv("stride");
    auto IC = getenv("n_ic");

    auto in_img = OX ? atoi(OX) : 40;
    auto ksize = KX ? atoi(KX) : 5;
    auto stride = S ? atoi(S) : 2;
    auto n_ic = IC ? atoi(IC) : 24;

    int X = in_img;
    int Y = in_img;
    int K_X = ksize;
    int K_Y = K_X;
    int C = n_ic;

    int num_glb_tiling = getenv("NUM_GLB_TILING") ? atoi(getenv("NUM_GLB_TILING")) : 1;
  
    // input image
    processor.inputs["input_host_stencil.mat"] = Buffer<uint16_t>(C, X, Y);
    processor.inputs_preset = true;
    auto input_copy_stencil = processor.inputs["input_host_stencil.mat"];
    for (int y = 0; y < input_copy_stencil.dim(2).extent(); y++) {
      for (int x = 0; x < input_copy_stencil.dim(1).extent(); x++) {
        for (int c = 0; c < input_copy_stencil.dim(0).extent(); c++) {
          input_copy_stencil(c, x, y) = float_to_bfloat16_process(((static_cast<float>(rand()) / RAND_MAX) * 4.0f) - 2.0f);
        }
      }
    }

    std::cout << "input has dims: " << processor.inputs["input_host_stencil.mat"].dim(0).extent() << "x"
              << processor.inputs["input_host_stencil.mat"].dim(1).extent() << "x"
              << processor.inputs["input_host_stencil.mat"].dim(2).extent() << "\n";

    // Full inputs
    auto input_host_stencil_full = Buffer<uint16_t>(num_glb_tiling * C, X, Y);
    for (int y = 0; y < input_host_stencil_full.dim(2).extent(); y++) {
      for (int x = 0; x < input_host_stencil_full.dim(1).extent(); x++) {
        for (int c = 0; c < input_host_stencil_full.dim(0).extent(); c++) {
          input_host_stencil_full(c, x, y) = float_to_bfloat16_process(((static_cast<float>(rand()) / RAND_MAX) * 4.0f) - 2.0f);
        }
      }
    }
    std::cout << "full input has dims: " << input_host_stencil_full.dim(0).extent() << "x"
              << input_host_stencil_full.dim(1).extent() << "x"
              << input_host_stencil_full.dim(2).extent() << "\n";


    int imgsize_x = std::floor( (X - K_X) / stride ) + 1;
    int imgsize_y = std::floor( (Y - K_Y) / stride ) + 1;
    processor.output = Buffer<uint16_t>(C, imgsize_x, imgsize_y);

    // Full output
    auto output_host_stencil_full = Buffer<uint16_t>(num_glb_tiling * C, imgsize_x, imgsize_y);
    std::cout << "full output has dims: " << output_host_stencil_full.dim(0).extent() << "x"
              << output_host_stencil_full.dim(1).extent() << "x"
              << output_host_stencil_full.dim(2).extent() << "\n";

    // Kernel generation similar to the preload kernel in the Halide generator
    int block_size = ksize;
    float step = 0.3f;
    // Assuming the kernel buffer dimensions are (C, block_size, block_size)
    Buffer<uint16_t> kernel_stencil(C, block_size, block_size);
    // Populate the kernel buffer
    for (int c = 0; c < C; ++c) {
        for (int y = 0; y < block_size; ++y) {
            for (int x = 0; x < block_size; ++x) {
                float value = 2.8f + step * (y * block_size + x);
                kernel_stencil(c, x, y) = float_to_bfloat16_process(value);
            }
        }
    }

    // Full kernels
    auto kernel_host_stencil_full = Buffer<uint16_t>(num_glb_tiling * C, K_X, K_Y);
    for (int y = 0; y < K_Y; y++) {
        for (int x = 0; x < K_X; x++) {
            for (int c = 0; c < num_glb_tiling * C; c++) {
                // Compute which channel in the original kernel to use
                int channel_index = c % C;
                // Copy value from the original kernel_stencil
                kernel_host_stencil_full(c, x, y) = kernel_stencil(channel_index, x, y);
            }
        }
    }

    // Depthwise Convolution Operation using the generated kernel
    for (int c = 0; c < num_glb_tiling * C; ++c) {
        for (int y = 0; y < imgsize_y; ++y) {
            for (int x = 0; x < imgsize_x; ++x) {
                float sum = 0.0f;
                for (int ky = 0; ky < K_Y; ++ky) {
                    for (int kx = 0; kx < K_X; ++kx) {
                        int ix = x * stride + kx;
                        int iy = y * stride + ky;
                        if (ix < X && iy < Y) {
                            float input_val = bfloat16_to_float_process(input_host_stencil_full(c, ix, iy));
                            float kernel_val = bfloat16_to_float_process(kernel_host_stencil_full(c, kx, ky)); // Use the generated kernel
                            sum += input_val * kernel_val;
                        }
                    }
                }
                output_host_stencil_full(c, x, y) = float_to_bfloat16_process(sum);
            }
        }
    }

    std::cout << "output has dims: " << processor.output.dim(0).extent() << "x"
              << processor.output.dim(1).extent() << "x"
              << processor.output.dim(2).extent() << "\n";
    
    bool write_mat = true;
    if (write_mat) {
      std::cout << "Writing input_host_stencil.mat to bin folder" << std::endl;
      std::cout << "Writing hw_output.mat to bin folder" << std::endl;
      save_image(input_host_stencil_full, "bin/input_host_stencil.mat");
      save_image(output_host_stencil_full, "bin/hw_output.mat");
    }

    // Generate glb_bank_config.json if "USE_GLB_BANK_CONFIG" is 1
    std::cout << "Checking for GLB bank configuration..." << std::endl;
    std::cout << "USE_GLB_BANK_CONFIG = " << getenv("USE_GLB_BANK_CONFIG") << std::endl;
    if (getenv("USE_GLB_BANK_CONFIG") && std::stoi(getenv("USE_GLB_BANK_CONFIG"))) {
      std::vector<int> input_host_stencil = parse_glb_bank_config_env_var("INPUT_HOST_STENCIL_POS");
      std::vector<int> kernel_host_stencil = parse_glb_bank_config_env_var("KERNEL_HOST_STENCIL_POS");
      std::vector<int> hw_output_stencil = parse_glb_bank_config_env_var("HW_OUTPUT_STENCIL_POS");
      std::vector<int> glb_inputs = parse_glb_bank_config_env_var("GLB_INPUTS");
      std::vector<int> num_glb_tiling_vec = parse_glb_bank_config_env_var("NUM_GLB_TILING");

      // Create the glb_bank_config.json structure
      json config = {
          {"inputs", {
              {"input_host_stencil", input_host_stencil},
              {"kernel_host_stencil", kernel_host_stencil}
          }},
          {"outputs", {
              {"hw_output", hw_output_stencil}
          }},
          {"glb_inputs", glb_inputs},
          {"num_glb_tiling", num_glb_tiling_vec}
      };

      std::ofstream file("bin/glb_bank_config.json");
      if (file.is_open()) {
          file << config.dump(4) << std::endl;
          file.close();
          std::cout << "Successfully wrote to bin/glb_bank_config.json" << std::endl;
      } else {
          std::cerr << "Unable to open file for writing." << std::endl;
          return 1;
      }
    }

    return processor.process_command(argc, argv);
}  
