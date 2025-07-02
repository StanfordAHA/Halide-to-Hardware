#include "coreir.h"
#include "halide_image_io.h"
#include "hardware_process_helper.h"
#include "hw_support_utils.h"
#include <cstdio>
#include <fstream>
#include <iostream>
#include <math.h>
#include <vector>

#if defined(WITH_CPU)
#include "get_e8m0_scale_test_fp.h"
#endif

#if defined(WITH_COREIR)
#include "coreir_interpret.h"
#endif

#if defined(WITH_CLOCKWORK)
#include "clockwork_sim_platform.h"
#include "get_e8m0_scale_test_fp_clockwork.h"
#include "rdai_api.h"
#endif

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main(int argc, char **argv) {
    std::map<std::string, std::function<void()>> functions;
    ManyInOneOut_ProcessController<uint16_t> processor("get_e8m0_scale_test_fp", { "input.mat", "input_2.mat" });

#if defined(WITH_CPU)
    auto cpu_process = [&](auto &proc) {
        get_e8m0_scale_test_fp(proc.inputs["input.mat"], proc.inputs["input_2.mat"], proc.output);
    };
    functions["cpu"] = [&]() {
        cpu_process(processor);
    };
#endif

#if defined(WITH_COREIR)
    auto coreir_process = [&](auto &proc) {
        run_coreir_on_interpreter<>("bin/design_top.json",
                                    proc.inputs["input.mat"], proc.output,
                                    "self.in_arg_0_0_0", "self.out_0_0");
    };
    functions["coreir"] = [&]() {
        coreir_process(processor);
    };
#endif

#if defined(WITH_CLOCKWORK)
    auto clockwork_process = [&](auto &proc) {
        RDAI_Platform *rdai_platform = RDAI_register_platform(&rdai_clockwork_sim_ops);
        if (rdai_platform) {
            printf("[RUN_INFO] found an RDAI platform\n");
            get_e8m0_scale_test_fp_clockwork(proc.inputs["input.mat"], proc.inputs["input_2.mat"], proc.output);
            RDAI_unregister_platform(rdai_platform);
        } else {
            printf("[RUN_INFO] failed to register RDAI platform!\n");
        }
    };
    functions["clockwork"] = [&]() {
        clockwork_process(processor);
    };
#endif

    processor.run_calls = functions;

    auto vec_height_env = getenv("vec_height");
    auto vec_width_env = getenv("vec_width");

    auto vec_height = vec_height_env ? atoi(vec_height_env) : 3136;
    auto vec_width = vec_width_env ? atoi(vec_width_env) : 32;

    // Assert that vec_height is even for proper bit packing
    assert((vec_height % 2 == 0) && "vec_height must be even for proper bit packing");

    std::cout << "using inputs set within process.cpp" << std::endl;
    processor.inputs_preset = true;

    // Input
    processor.inputs["input.mat"] = Buffer<uint16_t>(vec_width, vec_height);
    auto input_copy_stencil = processor.inputs["input.mat"];
    for (int y = 0; y < input_copy_stencil.dim(1).extent(); y++) {
        for (int x = 0; x < input_copy_stencil.dim(0).extent(); x++) {
            input_copy_stencil(x, y) = float_to_bfloat16_process((static_cast<float>(rand()) / RAND_MAX) * 128.0f - 64.0f);
        }
    }

    processor.inputs["input_2.mat"] = Buffer<uint16_t>(vec_width, vec_height);
    auto input_2_copy_stencil = processor.inputs["input_2.mat"];
    for (int y = 0; y < input_2_copy_stencil.dim(1).extent(); y++) {
        for (int x = 0; x < input_2_copy_stencil.dim(0).extent(); x++) {
            input_2_copy_stencil(x, y) = float_to_bfloat16_process((static_cast<float>(rand()) / RAND_MAX) * 128.0f - 64.0f);
        }
    }

    // Gold output
    processor.output = Buffer<uint16_t>(int(vec_height / 2));
    auto real_output = Buffer<uint16_t>(int(vec_height / 2));

    for (int y = 0; y < processor.inputs["input.mat"].dim(1).extent(); y++) {
        float max_val = 0.0f;
        uint16_t max_val_bf16 = float_to_bfloat16_process(max_val);
        for (int x = 0; x < processor.inputs["input.mat"].dim(0).extent(); x++) {
            for (int i = 0; i < processor.inputs["input_2.mat"].dim(0).extent(); i++) {
                max_val_bf16 = abs_max(max_val_bf16, processor.inputs["input_2.mat"](i, y));
            }
            max_val_bf16 = abs_max(max_val_bf16, processor.inputs["input.mat"](x, y));
        }
        // Pack every two 8-bit scale into one 16-bit word.
        real_output(int(y / 2)) = (y & 1) ? bit8_pack(get_shared_exp(max_val_bf16), real_output(int(y / 2))) : get_shared_exp(max_val_bf16);
    }

    saveHalideBufferToRawBigEndian(processor.inputs["input.mat"], "bin/mu_input_host_stencil.raw");
    saveHalideBufferToRawBigEndian(processor.inputs["input_2.mat"], "bin/glb_input_host_stencil.raw");
    saveHalideBufferToRawBigEndian(real_output, "bin/hw_output.raw");

    // Create glb bank config file for packing
    // Generate glb_bank_config.json if "USE_GLB_BANK_CONFIG" is 1
    std::cout << "Checking for GLB bank configuration..." << std::endl;
    std::cout << "USE_GLB_BANK_CONFIG = " << getenv("USE_GLB_BANK_CONFIG") << std::endl;
    if (getenv("USE_GLB_BANK_CONFIG") && std::stoi(getenv("USE_GLB_BANK_CONFIG"))) {
        std::vector<int> mu_input_host_stencil_pos = parse_glb_bank_config_num_list("MU_INPUT_HOST_STENCIL_POS");
        std::vector<int> glb_input_host_stencil_pos = parse_glb_bank_config_num_list("GLB_INPUT_HOST_STENCIL_POS");
        std::vector<int> hw_output_stencil_pos = parse_glb_bank_config_num_list("HW_OUTPUT_STENCIL_POS");

        std::vector<int> glb_input_host_stencil_packed = parse_glb_bank_config_num_list("GLB_INPUT_HOST_STENCIL_PACKED");
        std::vector<int> hw_output_stencil_packed = parse_glb_bank_config_num_list("HW_OUTPUT_STENCIL_PACKED");

        // Create the glb_bank_config.json structure
        json config = {
            {"mu_inputs", {
                {
                    {"mu_input_host_stencil", {
                        {"x_coord", mu_input_host_stencil_pos}
                    }}
                }
            }},
            {"inputs", {
                {
                    {"glb_input_host_stencil", {
                        {"x_coord", glb_input_host_stencil_pos},
                        {"E64_packed", glb_input_host_stencil_packed}
                    }}
                }
            }},
            {"outputs", {
                {
                    {"hw_output_stencil", {
                        {"x_coord", hw_output_stencil_pos},
                        {"E64_packed", hw_output_stencil_packed}
                    }}
                }
            }}
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

    auto output = processor.process_command(argc, argv);

    return output;
}