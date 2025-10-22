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
#include "fully_connected_layer_fp.h"
#endif

#if defined(WITH_COREIR)
#include "coreir_interpret.h"
#endif

#if defined(WITH_CLOCKWORK)
#include "clockwork_sim_platform.h"
#include "rdai_api.h"
#include "fully_connected_layer_fp_clockwork.h"
#endif

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main(int argc, char **argv) {
    std::map<std::string, std::function<void()>> functions;
    ManyInOneOut_ProcessController<uint16_t> processor("fully_connected_layer_fp", { "matrix", "vector"});

#if defined(WITH_CPU)
    auto cpu_process = [&](auto &proc) {
        fully_connected_layer_fp(proc.inputs["matrix"], proc.inputs["vector"], proc.output);
    };
    functions["cpu"] = [&]() {
        cpu_process(processor);
    };
#endif

#if defined(WITH_COREIR)
    auto coreir_process = [&](auto &proc) {
        run_coreir_on_interpreter<>("bin/design_top.json",
                                    proc.inputs["matrix"], proc.inputs["vector"], proc.output,
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
            fully_connected_layer_fp_clockwork(proc.inputs["matrix"], proc.inputs["vector"], proc.output);
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

    auto matrix_width_env = getenv("matrix_width");
    auto matrix_height_env = getenv("matrix_height");
    auto matrix_width_fake_env = getenv("matrix_width_fake");
    auto matrix_height_fake_env = getenv("matrix_height_fake");

    auto matrix_width = matrix_width_env ? atoi(matrix_width_env) : 32;
    auto matrix_height = matrix_height_env ? atoi(matrix_height_env) : 64;
    auto matrix_width_fake = matrix_width_fake_env ? atoi(matrix_width_fake_env) : 512;
    auto matrix_height_fake = matrix_height_fake_env ? atoi(matrix_height_fake_env) : 4;

    processor.inputs["matrix"] = Buffer<uint16_t>(matrix_width_fake, matrix_height_fake);
    processor.inputs["vector"] = Buffer<uint16_t>(matrix_width_fake);
    processor.output = Buffer<uint16_t>(matrix_height_fake);

    std::cout << "using inputs set within process.cpp" << std::endl;
    processor.inputs_preset = true;

    // Matrix
    auto real_matrix = Buffer<uint16_t>(matrix_width, matrix_height);
    for (int y = 0; y < real_matrix.dim(1).extent(); y++) {
        for (int x = 0; x < real_matrix.dim(0).extent(); x++) {
            // [-30.0, 30.0] with multiples of 0.5
            int steps = 121;
            int rnd_int = rand() % steps;
            float val = -30.0f + 0.5f * rnd_int;
            real_matrix(x, y) = float_to_bfloat16_process(val);
        }
    }

    // Vector
    auto real_vector = Buffer<uint16_t>(matrix_width);
    for (int x = 0; x < real_vector.dim(0).extent(); x++) {
        // [-30.0, 30.0] with multiples of 0.5
        int steps = 121;
        int rnd_int = rand() % steps;
        float val = -30.0f + 0.5f * rnd_int;
        real_vector(x) = float_to_bfloat16_process(val);
    }

    // Bias
    auto real_bias = Buffer<uint16_t>(matrix_height);
    for (int y = 0; y < real_bias.dim(0).extent(); y++) {
        int steps = 121;
        int rnd_int = rand() % steps;
        float val = -30.0f + 0.5f * rnd_int;
        real_bias(y) = float_to_bfloat16_process(val);
    }

    // Gold output
    auto real_output = Buffer<uint16_t>(matrix_height);

    // Calculate gold output
    for (int y = 0; y < int(matrix_height); y++) {
        float sum = 0.0f;
        for (int x = 0; x < int(matrix_width); x++) {
            float mat_val = bfloat16_to_float_process(real_matrix(x, y));
            float vec_val = bfloat16_to_float_process(real_vector(x));
            sum += mat_val * vec_val;
        }
        sum += bfloat16_to_float_process(real_bias(y));
        real_output(y) = float_to_bfloat16_process(sum);
    }

    saveHalideBufferToRawBigEndian(real_matrix, "bin/matrix_host_stencil.raw");
    saveHalideBufferToRawBigEndian(real_vector, "bin/vector_host_stencil.raw");
    saveHalideBufferToRawBigEndian(real_bias, "bin/bias_host_stencil.raw");
    saveHalideBufferToRawBigEndian(real_output, "bin/hw_output.raw");

    // Create glb bank config file for packing
    // Generate glb_bank_config.json if "USE_GLB_BANK_CONFIG" is 1
    std::cout << "Checking for GLB bank configuration..." << std::endl;
    std::cout << "USE_GLB_BANK_CONFIG = " << getenv("USE_GLB_BANK_CONFIG") << std::endl;
    if (getenv("USE_GLB_BANK_CONFIG") && std::stoi(getenv("USE_GLB_BANK_CONFIG"))) {
        std::vector<int> matrix_host_stencil_pos = parse_glb_bank_config_num_list("MATRIX_HOST_STENCIL_POS");
        std::vector<int> vector_host_stencil_pos = parse_glb_bank_config_num_list("VECTOR_HOST_STENCIL_POS");
        std::vector<int> bias_host_stencil_pos = parse_glb_bank_config_num_list("BIAS_HOST_STENCIL_POS");
        std::vector<int> hw_output_stencil_pos = parse_glb_bank_config_num_list("HW_OUTPUT_STENCIL_POS");

        std::vector<int> matrix_host_stencil_packed = parse_glb_bank_config_num_list("MATRIX_HOST_STENCIL_PACKED");
        std::vector<int> vector_host_stencil_packed = parse_glb_bank_config_num_list("VECTOR_HOST_STENCIL_PACKED");
        std::vector<int> bias_host_stencil_packed = parse_glb_bank_config_num_list("BIAS_HOST_STENCIL_PACKED");
        std::vector<int> hw_output_stencil_packed = parse_glb_bank_config_num_list("HW_OUTPUT_STENCIL_PACKED");

        std::vector<int> matrix_host_stencil_multi_bank_mode = parse_glb_bank_config_num_list("MATRIX_HOST_STENCIL_MULTI_BANK_MODE");
        std::vector<int> vector_host_stencil_multi_bank_mode = parse_glb_bank_config_num_list("VECTOR_HOST_STENCIL_MULTI_BANK_MODE");

        // Create the glb_bank_config.json structure
        json config = {
            {"inputs", {
                {
                    {"matrix_host_stencil", {
                        {"x_coord", matrix_host_stencil_pos},
                        {"E64_packed", matrix_host_stencil_packed},
                        {"use_multi_bank_mode", matrix_host_stencil_multi_bank_mode}
                    }}
                },
                {
                    {"vector_host_stencil", {
                        {"x_coord", vector_host_stencil_pos},
                        {"E64_packed", vector_host_stencil_packed},
                        {"use_multi_bank_mode", vector_host_stencil_multi_bank_mode}
                    }}
                },
                {
                    {"bias_host_stencil", {
                        {"x_coord", bias_host_stencil_pos},
                        {"E64_packed", bias_host_stencil_packed}
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