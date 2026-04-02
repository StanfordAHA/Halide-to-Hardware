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
#include "rms_norm_pass2_fp.h"
#endif

#if defined(WITH_COREIR)
#include "coreir_interpret.h"
#endif

#if defined(WITH_CLOCKWORK)
#include "clockwork_sim_platform.h"
#include "rdai_api.h"
#include "rms_norm_pass2_fp_clockwork.h"
#endif

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main(int argc, char **argv) {
    std::map<std::string, std::function<void()>> functions;
    ManyInOneOut_ProcessController<uint16_t> processor("rms_norm_pass2_fp", { "input", "weight", "output" });

#if defined(WITH_CPU)
    auto cpu_process = [&](auto &proc) {
        rms_norm_pass2_fp(proc.inputs["input"], proc.inputs["weight"], proc.output);
    };
    functions["cpu"] = [&]() {
        cpu_process(processor);
    };
#endif

#if defined(WITH_COREIR)
    auto coreir_process = [&](auto &proc) {
        run_coreir_on_interpreter<>("bin/design_top.json",
                                    proc.inputs["input"], proc.inputs["weight"], proc.output,
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
            rms_norm_pass2_fp_clockwork(proc.inputs["input"], proc.inputs["weight"], proc.output);
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

    auto vec_width_env = getenv("vec_width");
    auto vec_height_env = getenv("vec_height");

    auto vec_width = vec_width_env ? atoi(vec_width_env) : 384;
    auto vec_height = vec_height_env ? atoi(vec_height_env) : 128;

    std::cout << "using inputs set within process.cpp" << std::endl;
    processor.inputs_preset = true;


    auto real_input_env = getenv("USE_REAL_INPUT");
    bool use_random_tensors = true;
    if (real_input_env && strcmp(real_input_env, "1") == 0) {
        use_random_tensors = false;
    }


    auto real_input = Buffer<uint16_t>(vec_width, vec_height);
    for (int y = 0; y < real_input.dim(1).extent(); y++) {
        for (int x = 0; x < real_input.dim(0).extent(); x++) {
            real_input(x, y) = float_to_bfloat16_process((static_cast<float>(rand()) / RAND_MAX) * 20.0f - 10.0f);
        }
    }
    if (!use_random_tensors) {
        auto pass1_output_path_env = getenv("PASS1_OUTPUT_PATH");
        std::string pass1_output_path = "/aha/Halide-to-Hardware/apps/hardware_benchmarks/apps/rms_norm_pass2_fp/llama_prefill-rms_norm_pass1_gold/rms_norm_pass1_gold.raw";
        if (pass1_output_path_env) {
            pass1_output_path = std::string(pass1_output_path_env);
        }
        load_raw_to_halide_buffer(pass1_output_path, real_input);
        // Print the first few values of the input activation for debugging
        std::cout << "First few values of input activation:" << std::endl;
        for (int y = 0; y < std::min(2, real_input.dim(1).extent()); y++) {
            for (int x = 0; x < std::min(5, real_input.dim(0).extent()); x++) {
                std::cout << bfloat16_to_float_process(real_input(x, y)) << " ";
            }
            std::cout << std::endl;
        }
    }


    auto real_weight = Buffer<uint16_t>(vec_width);
    for (int x = 0; x < real_weight.dim(0).extent(); x++) {
        real_weight(x) = float_to_bfloat16_process((static_cast<float>(rand()) / RAND_MAX) * 20.0f - 10.0f);
    }
    if (!use_random_tensors) {
        load_raw_to_halide_buffer("hw_weight_stencil.raw", real_weight);
        // Print the first few values of the weight for debugging
        std::cout << "First few values of weight:" << std::endl;
        for (int x = 0; x < std::min(5, real_weight.dim(0).extent()); x++) {
            std::cout << bfloat16_to_float_process(real_weight(x)) << " ";
        }
        std::cout << std::endl;
    }

    // Real gold output: input * weight (gamma elementwise mul)
    auto real_output = Buffer<uint16_t>(vec_width, vec_height);
    for (int y = 0; y < real_output.dim(1).extent(); y++) {
        for (int x = 0; x < real_output.dim(0).extent(); x++) {
            float input_val = bfloat16_to_float_process(real_input(x, y));
            float weight_val = bfloat16_to_float_process(real_weight(x));
            float output_val = input_val * weight_val;
            real_output(x, y) = float_to_bfloat16_process(output_val);
        }
    }

    // Placeholder definitions for input and weight
    processor.inputs["input"] = Buffer<uint16_t>(vec_width, vec_height);
    processor.inputs["weight"] = Buffer<uint16_t>(vec_width, vec_height);
    processor.output = Buffer<uint16_t>(vec_width, vec_height);


    if (use_random_tensors) {
        // Use random tensors
        std::cout << "Generating random tensors" << std::endl;
        save_halide_buffer_to_raw(real_input, "bin/input_host_stencil.raw");
        save_halide_buffer_to_raw(real_weight, "bin/weight_host_stencil.raw");
    } else {
        int ret = 0;
        auto pass1_output_path_env = getenv("PASS1_OUTPUT_PATH");
        std::string pass1_output_path = "/aha/Halide-to-Hardware/apps/hardware_benchmarks/apps/rms_norm_pass2_fp/llama_prefill-rms_norm_pass1_gold/rms_norm_pass1_gold.raw";
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

        // Copy the pre-existing raw file into the bin folder for now
        ret = system("cp hw_weight_stencil.raw bin/weight_host_stencil.raw");
        if (ret != 0) {
            std::cerr << "Error: Failed to copy hw_weight_stencil.rawto bin folder. "
                        "hw_weight_stencil.raw should have been generated by /aha/voyager/scripts/aha_flow/parse_dnnLayer_tensors.py "
                        "(system call returned " << ret << ")" << std::endl;
            return 1;
        }
        std::cout << "Copying pre-existing hw_weight_stencil.raw to bin/weight_host_stencil.raw" << std::endl;

    }


    save_halide_buffer_to_raw(real_output, "bin/hw_output.raw");

    // Create glb bank config
    using namespace glb_cfg;
    // inputs, outputs, mu_inputs
    const config_spec spec = {
        {
            tensor_spec{"input_host_stencil", {"x_coord"}},
            tensor_spec{"weight_host_stencil", {"x_coord"}},
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