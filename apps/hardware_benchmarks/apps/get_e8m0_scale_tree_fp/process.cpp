#include "hardware_process_helper.h"
#include "halide_image_io.h"
#include "hw_support_utils.h"

#if defined(WITH_CPU)
#include "get_e8m0_scale_tree_fp.h"
#endif

#if defined(WITH_COREIR)
#include "coreir_interpret.h"
#endif

#if defined(WITH_CLOCKWORK)
#include "clockwork_sim_platform.h"
#include "get_e8m0_scale_tree_fp_clockwork.h"
#include "rdai_api.h"
#endif

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main(int argc, char **argv) {
    std::map<std::string, std::function<void()>> functions;
    ManyInOneOut_ProcessController<uint16_t> processor("get_e8m0_scale_tree_fp", { "mu_input.mat" });

#if defined(WITH_CPU)
    auto cpu_process = [&](auto &proc) {
        get_e8m0_scale_tree_fp(proc.inputs["mu_input.mat"], proc.output);
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
            get_e8m0_scale_tree_fp_clockwork(proc.inputs["mu_input.mat"], proc.output);
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
                // // Use [-10, 10], which is a typical range for activations of most models
                // real_mu_input(x, y, z) = float_to_bfloat16_process((static_cast<float>(rand()) / RAND_MAX) * 20.0f - 10.0f);
                // Use [-1024, 1024], which helps check scale output since otherwise scales are all the same
                real_mu_input(x, y, z) = float_to_bfloat16_process((static_cast<float>(rand()) / RAND_MAX) * 2048.0f - 1024.0f);
            }
        }
    }

    // Gold scale output
    processor.output = Buffer<uint16_t>(int(vec_height));
    auto real_output = Buffer<uint16_t>(int(vec_height), int(vec_width / block_size));
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

            // Store scale directly without packing
            uint16_t scale = get_shared_exp(max_val_bf16);
            real_output(y, b) = scale;
        }
    }

    // Unquantized output - just copy real_mu_input
    auto unquantized_output = Buffer<uint16_t>(mu_i, vec_height, int(vec_width / mu_i));
    for (int z = 0; z < real_mu_input.dim(2).extent(); z++) {
        for (int y = 0; y < real_mu_input.dim(1).extent(); y++) {
            for (int x = 0; x < real_mu_input.dim(0).extent(); x++) {
                unquantized_output(x, y, z) = real_mu_input(x, y, z);
            }
        }
    }

    // Save outputs
    save_halide_buffer_to_raw(real_mu_input, "bin/mu_input_host_stencil.raw");
    save_halide_buffer_to_raw(real_output, "bin/hw_scale_output.raw");
    save_halide_buffer_to_raw(unquantized_output, "bin/unquantized_output_stencil.raw");

    // Create glb bank config
    using namespace glb_cfg;
    // inputs, outputs, mu_inputs
    const config_spec spec = {
        {},
        {
            tensor_spec{"hw_scale_output_stencil", {"x_coord", "E64_packed"}},
            tensor_spec{"unquantized_output_stencil", {"x_coord"}}
        },
        {
            tensor_spec{"mu_input_host_stencil", {"x_coord"}}
        }
    };
    write_glb_bank_config(spec);

    auto output = processor.process_command(argc, argv);

    return output;
}