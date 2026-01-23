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
#include "layer_norm_pass2_fp.h"
#endif

#if defined(WITH_COREIR)
#include "coreir_interpret.h"
#endif

#if defined(WITH_CLOCKWORK)
#include "clockwork_sim_platform.h"
#include "rdai_api.h"
#include "layer_norm_pass2_fp_clockwork.h"
#endif

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main(int argc, char **argv) {
    std::map<std::string, std::function<void()>> functions;
    ManyInOneOut_ProcessController<uint16_t> processor("layer_norm_pass2_fp", { "input.mat" });

#if defined(WITH_CPU)
    auto cpu_process = [&](auto &proc) {
        layer_norm_pass2_fp(proc.inputs["input.mat"], proc.output);
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
            layer_norm_pass2_fp_clockwork(proc.inputs["input.mat"], proc.output);
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

    const float gamma = 1.2f;
    const float beta = -0.35f;

    // Input activation
    auto input_activation = Buffer<uint16_t>(vec_width, vec_height);
    for (int y = 0; y < input_activation.dim(1).extent(); y++) {
        for (int x = 0; x < input_activation.dim(0).extent(); x++) {
            input_activation(x, y) = float_to_bfloat16_process((static_cast<float>(rand()) / RAND_MAX) * 20.0f - 10.0f);
        }
    }

    // Sum input
    auto sum_input = Buffer<uint16_t>(vec_height);
    for (int y = 0; y < input_activation.dim(1).extent(); y++) {
        float sum_val = 0.0f;
        for (int x = 0; x < input_activation.dim(0).extent(); x++) {
            sum_val += bfloat16_to_float_process(input_activation(x, y));
        }
        sum_input(y) = float_to_bfloat16_process(sum_val);
    }

    // Pass 1 output x - sum(x)*(1/N)
    auto pass1_output = Buffer<uint16_t>(vec_width, vec_height);
    for (int y = 0; y < pass1_output.dim(1).extent(); y++) {
        for (int x = 0; x < pass1_output.dim(0).extent(); x++) {
            pass1_output(x, y) = float_to_bfloat16_process(
                bfloat16_to_float_process(input_activation(x, y)) + bfloat16_to_float_process(sum_input(y)) * (-1.0f / float(vec_width))
            );
        }
    }

    // Final gold layer norm output
    const float epsilon = 1e-5f;
    auto gold_output = Buffer<uint16_t>(vec_width, vec_height);
    std::vector<float> row_data(vec_width, 0.0f);

    for (int y = 0; y < gold_output.dim(1).extent(); y++) {
        // Load the row into float workspace so we can match PyTorch reference behavior
        for (int x = 0; x < gold_output.dim(0).extent(); x++) {
            row_data[x] = bfloat16_to_float_process(input_activation(x, y));
        }

        float mean = 0.0f;
        for (const float val : row_data) {
            mean += val;
        }
        mean /= static_cast<float>(vec_width);

        float variance = 0.0f;
        for (const float val : row_data) {
            const float diff = val - mean;
            variance += diff * diff;
        }
        variance /= static_cast<float>(vec_width);

        const float inv_std = 1.0f / sqrtf(variance + epsilon);
        for (int x = 0; x < gold_output.dim(0).extent(); x++) {
            const float normalized = (row_data[x] - mean) * inv_std;
            const float layer_norm_val = normalized * gamma + beta;
            gold_output(x, y) = float_to_bfloat16_process(layer_norm_val);
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

        int ret = 0;
        auto pass1_output_path_env = getenv("PASS1_OUTPUT_PATH");
        std::string pass1_output_path = "/aha/Halide-to-Hardware/apps/hardware_benchmarks/apps/layer_norm_pass2_fp/bert-layer_norm_pass1_gold/layer_norm_pass1_gold.raw";
        if (pass1_output_path_env) {
            pass1_output_path = std::string(pass1_output_path_env);
        }

        ret = system(("cp " + pass1_output_path + " bin/input_host_stencil.raw").c_str());
        if (ret != 0) {
            std::cerr << "Error: Failed to copy pass1 input raw to bin folder. "
                        "The pass1 input raw should have been produced by layer_norm_pass1 layer and saved by the user"
                        "(system call returned " << ret << ")" << std::endl;
            return 1;
        }
        std::cout << "Copying pre-existing " << pass1_output_path << " to bin/input_host_stencil.raw" << std::endl;
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