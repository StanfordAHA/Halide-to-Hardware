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
   #include "relu_layer_fp.h"
#endif

#if defined(WITH_COREIR)
    #include "coreir_interpret.h"
#endif

#if defined(WITH_CLOCKWORK)
    #include "rdai_api.h"
    #include "clockwork_sim_platform.h"
    #include "relu_layer_fp_clockwork.h"
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
  ManyInOneOut_ProcessController<uint16_t> processor("relu_layer_fp", {"hw_input_stencil.mat", "hw_bias_stencil.raw"});

  #if defined(WITH_CPU)
      auto cpu_process = [&]( auto &proc ) {
        relu_layer_fp(proc.inputs["hw_input_stencil.mat"], proc.inputs["hw_bias_stencil.raw"], proc.output);
      };
      functions["cpu"] = [&](){ cpu_process( processor ); } ;
  #endif

  #if defined(WITH_COREIR)
      auto coreir_process = [&]( auto &proc ) {
          run_coreir_on_interpreter<>( "bin/design_top.json",
                                       proc.inputs["hw_input_stencil.mat"], proc.output,
                                       "self.in_arg_0_0_0", "self.out_0_0" );
      };
      functions["coreir"] = [&](){ coreir_process( processor ); };
  #endif

  #if defined(WITH_CLOCKWORK)
      auto clockwork_process = [&]( auto &proc ) {
        RDAI_Platform *rdai_platform = RDAI_register_platform( &rdai_clockwork_sim_ops );
        if ( rdai_platform ) {
          printf( "[RUN_INFO] found an RDAI platform\n" );
          relu_layer_fp_clockwork(proc.inputs["hw_input_stencil.mat"], proc.inputs["hw_bias_stencil.raw"], proc.output);
          RDAI_unregister_platform( rdai_platform );
        } else {
          printf("[RUN_INFO] failed to register RDAI platform!\n");
        }
      };
      functions["clockwork"] = [&](){ clockwork_process( processor ); };
  #endif

  auto OX = getenv("out_img");
  auto OC = getenv("n_oc");

  auto out_img = OX ? atoi(OX) : 56;
  auto n_oc = OC ? atoi(OC) : 32;

  // Add all defined functions
  processor.run_calls = functions;

  processor.inputs["hw_input_stencil.mat"]        = Buffer<uint16_t>(n_oc, out_img, out_img);
  processor.inputs["hw_bias_stencil.raw" ]        = Buffer<uint16_t>(n_oc, out_img, out_img);
  processor.output                                = Buffer<uint16_t>(n_oc, out_img, out_img);

  processor.inputs_preset = true;

  for (int y = 0; y < processor.inputs["hw_input_stencil.mat"].dim(2).extent(); y++) {
    for (int x = 0; x < processor.inputs["hw_input_stencil.mat"].dim(1).extent(); x++) {
      for (int w = 0; w < processor.inputs["hw_input_stencil.mat"].dim(0).extent(); w++) {
        processor.inputs["hw_input_stencil.mat"](w, x, y) = float_to_bfloat16_process(
          // [-7, 7]
          ((float)rand() / RAND_MAX) * 14.0 - 7.0
        );
      }
    }
  }

  for (int y = 0; y < processor.inputs["hw_bias_stencil.raw"].dim(2).extent(); y++) {
    for (int x = 0; x < processor.inputs["hw_bias_stencil.raw"].dim(1).extent(); x++) {
      for (int w = 0; w < processor.inputs["hw_bias_stencil.raw"].dim(0).extent(); w++) {
        processor.inputs["hw_bias_stencil.raw"](w, x, y) = float_to_bfloat16_process(
          0.0
        );
      }
    }
  }

  // Gold output
  for (int w = 0; w < processor.output.dim(0).extent(); w++) {
    for (int x = 0; x < processor.output.dim(1).extent(); x++) {
      for (int y = 0; y < processor.output.dim(2).extent(); y++) {
        float sum = bfloat16_to_float_process(processor.inputs["hw_input_stencil.mat"](w, x, y)) + bfloat16_to_float_process(processor.inputs["hw_bias_stencil.raw"](w));
        float result = std::min(std::max(sum, 0.0f), 6.0f);
        processor.output(w, x, y) = float_to_bfloat16_process(result);
      }
    }
  }

  std::cout << "Writing hw_input_stencil.mat to bin folder" << std::endl;
  save_image(processor.inputs["hw_input_stencil.mat"], "bin/hw_input_stencil.mat");

  std::cout << "Writing hw_bias_stencil.raw to bin folder" << std::endl;
  saveHalideBufferToRawBigEndian(processor.inputs["hw_bias_stencil.raw"], "bin/hw_bias_stencil.raw");

  std::cout << "Writing output to bin folder" << std::endl;
  save_image(processor.output, "bin/hw_output.mat");

  // Generate glb_bank_config.json if "USE_GLB_BANK_CONFIG" is 1
  std::cout << "Checking for GLB bank configuration..." << std::endl;
  std::cout << "USE_GLB_BANK_CONFIG = " << getenv("USE_GLB_BANK_CONFIG") << std::endl;
  if (getenv("USE_GLB_BANK_CONFIG") && std::stoi(getenv("USE_GLB_BANK_CONFIG"))) {
    std::vector<int> hw_input_stencil = parse_glb_bank_config_env_var("HW_INPUT_STENCIL_POS");
    std::vector<int> hw_bias_stencil = parse_glb_bank_config_env_var("HW_BIAS_STENCIL_POS");
    std::vector<int> hw_output_stencil = parse_glb_bank_config_env_var("HW_OUTPUT_STENCIL_POS");
    std::vector<int> glb_inputs = parse_glb_bank_config_env_var("GLB_INPUTS");

    // Create the glb_bank_config.json structure
    json config = {
        {"inputs", {
            {"hw_input_stencil", hw_input_stencil},
            {"hw_bias_stencil", hw_bias_stencil}
        }},
        {"outputs", {
            {"hw_output_stencil", hw_output_stencil}
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
