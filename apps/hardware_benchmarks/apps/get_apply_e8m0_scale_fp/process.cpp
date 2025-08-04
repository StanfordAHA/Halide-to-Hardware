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
#include "get_apply_e8m0_scale_fp.h"
#endif

#if defined(WITH_COREIR)
#include "coreir_interpret.h"
#endif

#if defined(WITH_CLOCKWORK)
#include "clockwork_sim_platform.h"
#include "get_apply_e8m0_scale_fp_clockwork.h"
#include "rdai_api.h"
#endif

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main(int argc, char **argv) {
    std::map<std::string, std::function<void()>> functions;
    ManyInOneOut_ProcessController<uint16_t> processor("get_apply_e8m0_scale_fp", { "mu_input.mat" });

#if defined(WITH_CPU)
    auto cpu_process = [&](auto &proc) {
        get_apply_e8m0_scale_fp(proc.inputs["mu_input.mat"], proc.output);
    };
    functions["cpu"] = [&]() {
        cpu_process(processor);
    };
#endif

#if defined(WITH_COREIR)
    auto coreir_process = [&](auto &proc) {
        run_coreir_on_interpreter<>("bin/design_top.json",
                                    proc.inputs["mu_input.mat"], proc.output,
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
            get_apply_e8m0_scale_fp_clockwork(proc.inputs["mu_input.mat"], proc.output);
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
    auto vec_width_fake_env = getenv("vec_width_fake");
    auto vec_width_env = getenv("vec_width");
    auto mu_i_env = getenv("mu_i");

    auto vec_height = vec_height_env ? atoi(vec_height_env) : 3136;
    auto vec_width_fake = vec_width_fake_env ? atoi(vec_width_fake_env) : 32;
    auto vec_width = vec_width_env ? atoi(vec_width_env) : 64;
    auto mu_i = mu_i_env ? atoi(mu_i_env) : 32;
    const int block_size = 64;

    // Assert that vec_height is even for proper bit packing
    assert((vec_height % 2 == 0) && "vec_height must be even for proper bit packing");

    std::cout << "using inputs set within process.cpp" << std::endl;
    processor.inputs_preset = true;

    // Input
    processor.inputs["mu_input.mat"] = Buffer<uint16_t>(vec_width_fake, vec_height);
    auto real_mu_input = Buffer<uint16_t>(mu_i, vec_height, int(vec_width / mu_i));
    for (int z = 0; z < real_mu_input.dim(2).extent(); z++) {
        for (int y = 0; y < real_mu_input.dim(1).extent(); y++) {
            for (int x = 0; x < real_mu_input.dim(0).extent(); x++) {
                real_mu_input(x, y, z) = float_to_bfloat16_process((static_cast<float>(rand()) / RAND_MAX) * 32768.0f);
            }
        }
    }

    // Gold output
    // processor.output = Buffer<uint16_t>(int(vec_height / 2));
    // auto real_output = Buffer<uint16_t>(int(vec_height / 2));

    processor.output = Buffer<uint16_t>(vec_height);
    auto real_output = Buffer<uint16_t>(vec_height, int(vec_width / block_size));

    for (int y = 0; y < real_mu_input.dim(1).extent(); y++) {
        // For each block along width (e.g., block_size=64, vec_width=128 -> 2 blocks)
        for (int b = 0; b < int(vec_width / block_size); b++) {
            float max_val = 0.0f;
            uint16_t max_val_bf16 = float_to_bfloat16_process(max_val);

            // For channels in the block (e.g., 64 channels)
            for (int ch = 0; ch < block_size; ch++) {
                int channel_idx = b * block_size + ch;
                int x = channel_idx % mu_i;
                int z = channel_idx / mu_i;
                if (z < real_mu_input.dim(2).extent() && x < real_mu_input.dim(0).extent()) {
                    max_val_bf16 = abs_max(max_val_bf16, real_mu_input(x, y, z));
                }
            }

            real_output(y, b) = get_shared_exp(max_val_bf16);

        // // Pack every two 8-bit scale into one 16-bit word.
        // real_output(int(y / 2)) = (y & 1) ? bit8_pack(get_shared_exp(max_val_bf16), real_output(int(y / 2))) : get_shared_exp(max_val_bf16);
        }
    }

    saveHalideBufferToRawBigEndian(real_mu_input, "bin/mu_input_host_stencil.raw");
    saveHalideBufferToRawBigEndian(real_output, "bin/hw_output.raw");

    // // Create glb bank config file for packing
    // // Generate glb_bank_config.json if "USE_GLB_BANK_CONFIG" is 1
    // std::cout << "Checking for GLB bank configuration..." << std::endl;
    // std::cout << "USE_GLB_BANK_CONFIG = " << getenv("USE_GLB_BANK_CONFIG") << std::endl;
    // if (getenv("USE_GLB_BANK_CONFIG") && std::stoi(getenv("USE_GLB_BANK_CONFIG"))) {
    //     std::vector<int> mu_input_host_stencil_pos = parse_glb_bank_config_num_list("MU_INPUT_HOST_STENCIL_POS");
    //     std::vector<int> glb_input_host_stencil_pos = parse_glb_bank_config_num_list("GLB_INPUT_HOST_STENCIL_POS");
    //     std::vector<int> hw_output_stencil_pos = parse_glb_bank_config_num_list("HW_OUTPUT_STENCIL_POS");

    //     std::vector<int> glb_input_host_stencil_packed = parse_glb_bank_config_num_list("GLB_INPUT_HOST_STENCIL_PACKED");
    //     std::vector<int> hw_output_stencil_packed = parse_glb_bank_config_num_list("HW_OUTPUT_STENCIL_PACKED");
    //     std::vector<int> hw_output_stencil_bank_toggle_mode = parse_glb_bank_config_num_list("HW_OUTPUT_STENCIL_BANK_TOGGLE_MODE");

    //     // Create the glb_bank_config.json structure
    //     json config = {
    //         {"mu_inputs", {
    //             {
    //                 {"mu_input_host_stencil", {
    //                     {"x_coord", mu_input_host_stencil_pos}
    //                 }}
    //             }
    //         }},
    //         {"inputs", {
    //             {
    //                 {"glb_input_host_stencil", {
    //                     {"x_coord", glb_input_host_stencil_pos},
    //                     {"E64_packed", glb_input_host_stencil_packed}
    //                 }}
    //             }
    //         }},
    //         {"outputs", {
    //             {
    //                 {"hw_output_stencil", {
    //                     {"x_coord", hw_output_stencil_pos},
    //                     {"E64_packed", hw_output_stencil_packed},
    //                     {"bank_toggle_mode", hw_output_stencil_bank_toggle_mode}
    //                 }}
    //             }
    //         }}
    //     };

    //     std::ofstream file("bin/glb_bank_config.json");
    //     if (file.is_open()) {
    //         file << config.dump(4) << std::endl;
    //         file.close();
    //         std::cout << "Successfully wrote to bin/glb_bank_config.json" << std::endl;
    //     } else {
    //         std::cerr << "Unable to open file for writing." << std::endl;
    //         return 1;
    //     }
    // }

    auto output = processor.process_command(argc, argv);

    return output;
}