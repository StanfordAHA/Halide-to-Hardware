#include "hardware_process_helper.h"
#include "halide_image_io.h"
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

    // // Check for gold tensors
    // bool use_resnet_gold = std::filesystem::exists("resnet18_gold_tensors/matrix_host_stencil_0.raw");

    // if (use_resnet_gold) {
    //     std::cout << "Using ResNet18 gold tensors" << std::endl;
    //     // Copy gold tensors to bin directory
    //     std::filesystem::copy_file(
    //         "resnet18_gold_tensors/matrix_host_stencil_0.raw",
    //         "bin/matrix_host_stencil.raw",
    //         std::filesystem::copy_options::overwrite_existing
    //     );
    //     std::filesystem::copy_file(
    //         "resnet18_gold_tensors/vector_host_stencil.raw",
    //         "bin/vector_host_stencil.raw",
    //         std::filesystem::copy_options::overwrite_existing
    //     );
    //     std::filesystem::copy_file(
    //         "resnet18_gold_tensors/bias_host_stencil_0.raw",
    //         "bin/bias_host_stencil.raw",
    //         std::filesystem::copy_options::overwrite_existing
    //     );
    //     std::filesystem::copy_file(
    //         "resnet18_gold_tensors/hw_output_0.raw",
    //         "bin/hw_output.raw",
    //         std::filesystem::copy_options::overwrite_existing
    //     );
    // } else {
    //     std::cout << "Generating random tensors" << std::endl;
    //     save_halide_buffer_to_raw(real_matrix, "bin/matrix_host_stencil.raw");
    //     save_halide_buffer_to_raw(real_vector, "bin/vector_host_stencil.raw");
    //     save_halide_buffer_to_raw(real_bias, "bin/bias_host_stencil.raw");
    //     save_halide_buffer_to_raw(real_output, "bin/hw_output.raw");
    // }

    // Copy the pre-existing raw file into the bin folder for now
    int ret = system("cp hw_input_stencil.raw bin/vector_host_stencil.raw");
    if (ret != 0) {
        std::cerr << "Error: Failed to copy hw_input_stencil.raw to bin folder. "
                    "hw_input_stencil.raw should have been generated by /aha/voyager/scripts/aha_flow.parse_dnnLayer_tensors.py "
                    "(system call returned " << ret << ")" << std::endl;
        return 1;
    }
    std::cout << "Copying pre-existing hw_input_stencil.raw to bin/vector_host_stencil.raw" << std::endl;

    // Copy the pre-existing raw file into the bin folder for now
    int ret1 = system("cp hw_weight_stencil.raw bin/matrix_host_stencil.raw");
    if (ret1 != 0) {
        std::cerr << "Error: Failed to copy hw_weight_stencil.raw to bin folder. "
                    "hw_weight_stencil.raw should have been generated by /aha/voyager/scripts/aha_flow.parse_dnnLayer_tensors.py "
                    "(system call returned " << ret1 << ")" << std::endl;
        return 1;
    }
    std::cout << "Copying pre-existing hw_weight_stencil.raw to bin/matrix_host_stencil.raw" << std::endl;

    // Copy the pre-existing raw file into the bin folder for now
    int ret2 = system("cp hw_bias_stencil.raw bin/bias_host_stencil.raw");
    if (ret2 != 0) {
        std::cerr << "Error: Failed to copy hw_bias_stencil.raw to bin folder. "
                    "hw_bias_stencil.raw should have been generated by /aha/voyager/scripts/aha_flow.parse_dnnLayer_tensors.py "
                    "(system call returned " << ret2 << ")" << std::endl;
        return 1;
    }
    std::cout << "Copying pre-existing hw_bias_stencil.raw to bin/bias_host_stencil.raw" << std::endl;


   std::cout << "Writing output to bin folder" << std::endl;
   save_halide_buffer_to_raw(processor.output, "bin/hw_output.raw");

    // Create glb bank config
    using namespace glb_cfg;
    // inputs, outputs, mu_inputs
    const config_spec spec = {
        {
            tensor_spec{"matrix_host_stencil", {"x_coord"}},
            tensor_spec{"vector_host_stencil", {"x_coord"}},
            tensor_spec{"bias_host_stencil", {"x_coord", "E64_packed"}}
        },
        {
            tensor_spec{"hw_output", {"x_coord", "E64_packed"}}
        },
        {}
    };
    write_glb_bank_config(spec);

    auto output = processor.process_command(argc, argv);

    return output;
}