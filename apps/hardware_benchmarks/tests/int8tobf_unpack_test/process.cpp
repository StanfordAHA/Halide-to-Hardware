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
#include "int8tobf_unpack_test.h"
#endif

#if defined(WITH_COREIR)
#include "coreir_interpret.h"
#endif

#if defined(WITH_CLOCKWORK)
#include "int8tobf_unpack_test_clockwork.h"
#include "clockwork_sim_platform.h"
#include "rdai_api.h"
#endif

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main(int argc, char **argv) {
    std::map<std::string, std::function<void()>> functions;
    ManyInOneOut_ProcessController<uint16_t> processor("int8tobf_unpack_test", { "hw_input_stencil" });

#if defined(WITH_CPU)
    auto cpu_process = [&](auto &proc) {
        int8tobf_unpack_test(proc.inputs["hw_input_stencil"], proc.output);
    };
    functions["cpu"] = [&]() { cpu_process(processor); };
#endif

#if defined(WITH_COREIR)
    auto coreir_process = [&](auto &proc) {
        run_coreir_on_interpreter<>("bin/design_top.json",
                                    proc.inputs["hw_input_stencil"], proc.output,
                                    "self.in_arg_0_0_0", "self.out_0_0");
    };
    functions["coreir"] = [&]() { coreir_process(processor); };
#endif

#if defined(WITH_CLOCKWORK)
    auto clockwork_process = [&](auto &proc) {
        RDAI_Platform *rdai_platform = RDAI_register_platform(&rdai_clockwork_sim_ops);
        if (rdai_platform) {
            printf("[RUN_INFO] found an RDAI platform\n");
            int8tobf_unpack_test_clockwork(proc.inputs["hw_input_stencil"], proc.output);
            RDAI_unregister_platform(rdai_platform);
        } else {
            printf("[RUN_INFO] failed to register RDAI platform!\n");
        }
    };
    functions["clockwork"] = [&]() { clockwork_process(processor); };
#endif

    auto OX = getenv("out_img");
    auto OC = getenv("n_oc");

    auto out_img = OX ? atoi(OX) : 8;
    auto n_oc = OC ? atoi(OC) : 16;

    // Add all defined functions
    processor.run_calls = functions;

    processor.inputs["hw_input_stencil"] = Buffer<uint16_t>(n_oc, out_img, out_img);
    processor.output = Buffer<uint16_t>(n_oc, out_img, out_img);

    processor.inputs_preset = true;

    // First define random buffer for generating packed int8 samples
    auto rnd_bf16_buffer = Buffer<uint16_t>(n_oc, out_img, out_img);
    for (int y = 0; y < rnd_bf16_buffer.dim(2).extent(); y++) {
        for (int x = 0; x < rnd_bf16_buffer.dim(1).extent(); x++) {
            for (int w = 0; w < rnd_bf16_buffer.dim(0).extent(); w++) {
                rnd_bf16_buffer(w, x, y) = float_to_bfloat16_process(
                    // [-63.0, 63.0]
                    ((float) rand() / RAND_MAX) * 126.0 - 63.0);
            }
        }
    }

    // Inputs: packed int8 samples from random bf16 buffer
    for (int w = 0; w < processor.output.dim(0).extent(); w++) {
        for (int x = 0; x < processor.output.dim(1).extent(); x++) {
            for (int y = 0; y < processor.output.dim(2).extent(); y++) {
                float mult_f32 = bfloat16_to_float_process(rnd_bf16_buffer(w, x, y)) * (-2.0f);
                uint16_t mult_bf16 = float_to_bfloat16_process(mult_f32);
                uint16_t result = bf16toint8_pack(rnd_bf16_buffer(w, x, y), mult_bf16);
                processor.inputs["hw_input_stencil"](w, x, y) = result;
            }
        }
    }

    // Gold output
    for (int w = 0; w < processor.output.dim(0).extent(); w++) {
        for (int x = 0; x < processor.output.dim(1).extent(); x++) {
            for (int y = 0; y < processor.output.dim(2).extent(); y++) {
                uint16_t unpack_high = int8tobf16_unpack_high(processor.inputs["hw_input_stencil"](w, x, y));
                uint16_t unpack_low = int8tobf16_unpack_low(processor.inputs["hw_input_stencil"](w, x, y));
                float unpack_a = bfloat16_to_float_process(unpack_high);
                float unpack_b = bfloat16_to_float_process(unpack_low);
                float mult_f32 = unpack_a * unpack_b;
                uint16_t result = float_to_bfloat16_process(mult_f32);
                processor.output(w, x, y) = result;
            }
        }
    }

    std::cout << "Writing hw_input_stencil to bin folder" << std::endl;
    save_halide_buffer_to_raw(processor.inputs["hw_input_stencil"], "bin/hw_input_stencil.raw");

    std::cout << "Writing output to bin folder" << std::endl;
    save_halide_buffer_to_raw(processor.output, "bin/hw_output.raw");

    return processor.process_command(argc, argv);
}
