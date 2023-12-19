#include <iostream>
#include <math.h>
#include <cstdio>
#include "hardware_process_helper.h"
#include "halide_image_io.h"

#if defined(WITH_CPU)
#include "matrix_multiplication.h"
#endif

#if defined(WITH_COREIR)
#include "coreir_interpret.h"
#endif

#if defined(WITH_CLOCKWORK)
#include "rdai_api.h"
#include "clockwork_sim_platform.h"
#include "matrix_multiplication_clockwork.h"
#endif

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main(int argc, char ** argv) {
    std::map < std::string, std:: function < void() >> functions;
    ManyInOneOut_ProcessController < int16_t > processor("matrix_multiplication", {
        "input.mat",
        "kernel.mat"
    });

    #if defined(WITH_CPU)
    auto cpu_process = [ & ](auto & proc) {
        matrix_multiplication(proc.inputs["input.mat"], proc.inputs["kernel.mat"], proc.output);
    };
    functions["cpu"] = [ & ]() {
        cpu_process(processor);
    };
    #endif

    #if defined(WITH_COREIR)
    auto coreir_process = [ & ](auto & proc) {
        run_coreir_on_interpreter < > ("bin/design_top.json",
            proc.inputs["input.mat"], proc.output,
            "self.in_arg_0_0_0", "self.out_0_0");
    };
    functions["coreir"] = [ & ]() {
        coreir_process(processor);
    };
    #endif

    #if defined(WITH_CLOCKWORK)
    auto clockwork_process = [ & ](auto & proc) {
        RDAI_Platform * rdai_platform = RDAI_register_platform( & rdai_clockwork_sim_ops);
        if (rdai_platform) {
            printf("[RUN_INFO] found an RDAI platform\n");
            matrix_multiplication_clockwork(proc.inputs["input.mat"], proc.inputs["kernel.mat"], proc.output);
            RDAI_unregister_platform(rdai_platform);
        } else {
            printf("[RUN_INFO] failed to register RDAI platform!\n");
        }
    };
    functions["clockwork"] = [ & ]() {
        clockwork_process(processor);
    };
    #endif

    processor.run_calls = functions;

    auto input_height_env = getenv("input_height");
    auto input_width_env = getenv("input_width");
    auto kernel_width_env = getenv("kernel_width");

    auto input_height = input_height_env ? atoi(input_height_env) : 64;
    auto input_width = input_width_env ? atoi(input_width_env) : 8;
    auto kernel_width = kernel_width_env ? atoi(kernel_width_env) : 8;

    std::cout << "using inputs set within process.cpp" << std::endl;
    processor.inputs_preset = true;


    ///// INPUT IMAGE /////
    processor.inputs["input.mat"] = Buffer < int16_t > (input_width, input_height);
    auto input_copy_stencil = processor.inputs["input.mat"];
    int max_rand = pow(2, 16) - 1;
    for (int y = 0; y < input_copy_stencil.dim(1).extent(); y++) {
        for (int x = 0; x < input_copy_stencil.dim(0).extent(); x++) {
            input_copy_stencil(x, y) = (rand() % max_rand);
        }
    }


    ///// KERNEL WEIGHTS /////  
    processor.inputs["kernel.mat"] = Buffer < int16_t > (kernel_width, input_width);
    auto kernel_copy_stencil = processor.inputs["kernel.mat"];
    for (int z = 0; z < kernel_copy_stencil.dim(1).extent(); z++) {
        for (int w = 0; w < kernel_copy_stencil.dim(0).extent(); w++) {
            kernel_copy_stencil(w, z) = (rand() % max_rand);
        }
    }

    save_image(processor.inputs["input.mat"], "bin/input.mat");
    save_image(processor.inputs["kernel.mat"], "bin/kernel.mat");

    processor.output = Buffer < int16_t > (kernel_width, input_height);

    auto output = processor.process_command(argc, argv);


    return output;
}