#include <iostream>
#include <math.h>
#include <cstdio>
#include <fstream>
#include <vector>
#include "hardware_process_helper.h"
#include "halide_image_io.h"
#include "coreir.h"
#include "hw_support_utils.h"

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

    // input image
    processor.inputs["input_host_stencil.mat"] = Buffer<uint16_t>(C, X, Y);
    processor.inputs_preset = true;
    auto input_copy_stencil = processor.inputs["input_host_stencil.mat"];
    for (int y = 0; y < input_copy_stencil.dim(2).extent(); y++) {
      for (int x = 0; x < input_copy_stencil.dim(1).extent(); x++) {
        for (int c = 0; c < input_copy_stencil.dim(0).extent(); c++) {
          input_copy_stencil(c, x, y) = float_to_bfloat16_process((static_cast<float>(rand()) / RAND_MAX) * 6.0f);
        }
      }
    }

    std::cout << "input has dims: " << processor.inputs["input_host_stencil.mat"].dim(0).extent() << "x"
              << processor.inputs["input_host_stencil.mat"].dim(1).extent() << "x"
              << processor.inputs["input_host_stencil.mat"].dim(2).extent() << "\n";


    int imgsize_x = std::floor( (X - K_X) / stride ) + 1;
    int imgsize_y = std::floor( (Y - K_Y) / stride ) + 1;
    processor.output = Buffer<uint16_t>(C, imgsize_x, imgsize_y);

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
    // Depthwise Convolution Operation using the generated kernel
    for (int c = 0; c < C; ++c) {
        for (int y = 0; y < imgsize_y; ++y) {
            for (int x = 0; x < imgsize_x; ++x) {
                float sum = 0.0f;
                for (int ky = 0; ky < K_Y; ++ky) {
                    for (int kx = 0; kx < K_X; ++kx) {
                        int ix = x * stride + kx;
                        int iy = y * stride + ky;
                        if (ix < X && iy < Y) {
                            float input_val = bfloat16_to_float_process(input_copy_stencil(c, ix, iy));
                            float kernel_val = bfloat16_to_float_process(kernel_stencil(c, kx, ky)); // Use the generated kernel
                            sum += input_val * kernel_val;
                        }
                    }
                }
                processor.output(c, x, y) = float_to_bfloat16_process(sum);
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
      save_image(processor.inputs["input_host_stencil.mat"], "bin/input_host_stencil.mat");
      save_image(processor.output, "bin/hw_output.mat");
    }

    // Generate glb_bank_config.json if "USE_GLB_BANK_CONFIG" is 1
    std::cout << "Checking for GLB bank configuration..." << std::endl;
    std::cout << "USE_GLB_BANK_CONFIG = " << getenv("USE_GLB_BANK_CONFIG") << std::endl;
    if (getenv("USE_GLB_BANK_CONFIG") && std::stoi(getenv("USE_GLB_BANK_CONFIG"))) {
      std::vector<int> input_host_stencil = parse_glb_bank_config_env_var("INPUT_HOST_STENCIL_POS");
      std::vector<int> kernel_host_stencil = parse_glb_bank_config_env_var("KERNEL_HOST_STENCIL_POS");
      std::vector<int> hw_output_stencil = parse_glb_bank_config_env_var("HW_OUTPUT_STENCIL_POS");
      std::vector<int> glb_inputs = parse_glb_bank_config_env_var("GLB_INPUTS");

      // Create the glb_bank_config.json structure
      json config = {
          {"inputs", {
              {"input_host_stencil", input_host_stencil},
              {"kernel_host_stencil", kernel_host_stencil}
          }},
          {"outputs", {
              {"hw_output", hw_output_stencil}
          }},
          {"glb_inputs", glb_inputs}
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
