#include "coreir.h"
#include "halide_image_io.h"
#include "hardware_process_helper.h"
#include <cstdio>
#include <fstream>
#include <iostream>
#include <math.h>
#include <vector>
#include "hw_support_utils.h"

#if defined(WITH_CPU)
#include "rms_norm_pass1_fp.h"
#endif

#if defined(WITH_COREIR)
#include "coreir_interpret.h"
#endif

#if defined(WITH_CLOCKWORK)
#include "clockwork_sim_platform.h"
#include "rdai_api.h"
#include "rms_norm_pass1_fp_clockwork.h"
#endif

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main(int argc, char **argv) {
    std::map<std::string, std::function<void()>> functions;
    ManyInOneOut_ProcessController<uint16_t> processor("rms_norm_pass1_fp", { "input.mat" });

#if defined(WITH_CPU)
    auto cpu_process = [&](auto &proc) {
        rms_norm_pass1_fp(proc.inputs["input.mat"], proc.output);
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
            rms_norm_pass1_fp_clockwork(proc.inputs["input.mat"], proc.output);
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

    auto vec_width_fake_env = getenv("vec_width_fake");
    auto vec_height_fake_env = getenv("vec_height_fake");
    auto vec_width_env = getenv("vec_width");
    auto vec_height_env = getenv("vec_height");

    auto vec_width_fake = vec_width_fake_env ? atoi(vec_width_fake_env) : 2048;
    auto vec_height_fake = vec_height_fake_env ? atoi(vec_height_fake_env) : 4;
    auto vec_width = vec_width_env ? atoi(vec_width_env) : 768;
    auto vec_height = vec_height_env ? atoi(vec_height_env) : 128;

    std::cout << "using inputs set within process.cpp" << std::endl;
    processor.inputs_preset = true;

    // Input activation
    auto input_activation = Buffer<uint16_t>(vec_width, vec_height);
    for (int y = 0; y < input_activation.dim(1).extent(); y++) {
        for (int x = 0; x < input_activation.dim(0).extent(); x++) {
            input_activation(x, y) = float_to_bfloat16_process((static_cast<float>(rand()) / RAND_MAX) * 20.0f - 10.0f);
        }
    }

    // Gold: pass 1 intermediate output — input normalized by RMS (no weight applied)
    auto gold_output = Buffer<uint16_t>(vec_width, vec_height);

    for (int y = 0; y < gold_output.dim(1).extent(); y++) {
        float sum_sq = 0.0f;
        for (int x = 0; x < vec_width; x++) {
            const float val = bfloat16_to_float_process(input_activation(x, y));
            sum_sq += val * val;
        }
        const float rms = sqrtf(sum_sq / static_cast<float>(vec_width));

        for (int x = 0; x < gold_output.dim(0).extent(); x++) {
            const float val = bfloat16_to_float_process(input_activation(x, y));
            gold_output(x, y) = float_to_bfloat16_process(val / rms);
        }
    }

    // Define fake processor input and output placeholer buffers
    processor.inputs["input.mat"] = Buffer<uint16_t>(vec_width_fake, vec_height_fake);
    processor.output = Buffer<uint16_t>(vec_width_fake, vec_height_fake);

    auto real_input_env = getenv("USE_REAL_INPUT");
    bool use_random_tensors = true;
    if (real_input_env && strcmp(real_input_env, "1") == 0) {
        use_random_tensors = false;
    }

    if (use_random_tensors) {
        // Use random tensors
        std::cout << "Generating random tensors" << std::endl;
        save_halide_buffer_to_raw(input_activation, "bin/input_host_stencil.raw");
    } else {

        // int ret = 0;
        // auto pass1_output_path_env = getenv("PASS1_OUTPUT_PATH");
        // std::string pass1_output_path = "/aha/Halide-to-Hardware/apps/hardware_benchmarks/apps/rms_norm_pass1_fp/bert-layer_norm_pass1_gold/layer_norm_pass1_gold.raw";
        // if (pass1_output_path_env) {
        //     pass1_output_path = std::string(pass1_output_path_env);
        // }

        // ret = system(("cp " + pass1_output_path + " bin/input_host_stencil.raw").c_str());
        // if (ret != 0) {
        //     std::cerr << "Error: Failed to copy pass1 input raw to bin folder. "
        //                 "The pass1 input raw should have been produced by layer_norm_pass1 layer and saved by the user"
        //                 "(system call returned " << ret << ")" << std::endl;
        //     return 1;
        // }
        // std::cout << "Copying pre-existing " << pass1_output_path << " to bin/input_host_stencil.raw" << std::endl;
    }
    save_halide_buffer_to_raw(gold_output, "bin/hw_output.raw");

    // Create glb bank config
    using namespace glb_cfg;
    // inputs, outputs, mu_inputs
    const config_spec spec = {
        {
            tensor_spec{"input_host_stencil", {"x_coord"}},
        },
        {
            tensor_spec{"hw_output", {"x_coord"}}
        },
        {}
    };
    write_glb_bank_config(spec);

    auto output = processor.process_command(argc, argv);

    return output;
}