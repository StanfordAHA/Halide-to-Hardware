#include "hardware_process_helper.h"
#include "halide_image_io.h"
#include "hw_support_utils.h"

#if defined(WITH_CPU)
#include "mat_vec_mul_fp.h"
#endif

#if defined(WITH_COREIR)
#include "coreir_interpret.h"
#endif

#if defined(WITH_CLOCKWORK)
#include "clockwork_sim_platform.h"
#include "rdai_api.h"
#include "mat_vec_mul_fp_clockwork.h"
#endif

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main(int argc, char **argv) {
    std::map<std::string, std::function<void()>> functions;
    ManyInOneOut_ProcessController<uint16_t> processor("mat_vec_mul_fp", { "matrix", "vector" });

#if defined(WITH_CPU)
    auto cpu_process = [&](auto &proc) {
        mat_vec_mul_fp(proc.inputs["matrix"], proc.inputs["vector"], proc.output);
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
            mat_vec_mul_fp_clockwork(proc.inputs["matrix"], proc.inputs["vector"], proc.output);
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
        real_output(y) = float_to_bfloat16_process(sum);
    }

    save_halide_buffer_to_raw(real_matrix, "bin/matrix_host_stencil.raw");
    save_halide_buffer_to_raw(real_vector, "bin/vector_host_stencil.raw");
    save_halide_buffer_to_raw(real_output, "bin/hw_output.raw");

    // Create glb bank config
    using namespace glb_cfg;
    // inputs, outputs, mu_inputs
    const config_spec spec = {
        {
            tensor_spec{"matrix_host_stencil", {"x_coord", "E64_packed", "use_multi_bank_mode"}},
            tensor_spec{"vector_host_stencil", {"x_coord", "E64_packed", "use_multi_bank_mode"}}
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