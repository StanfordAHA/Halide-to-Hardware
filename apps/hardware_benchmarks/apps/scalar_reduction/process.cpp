#include "coreir.h"
#include "halide_image_io.h"
#include "hardware_process_helper.h"
#include <cstdio>
#include <fstream>
#include <iostream>
#include <math.h>
#include <vector>

#if defined(WITH_CPU)
#include "scalar_reduction.h"
#endif

#if defined(WITH_COREIR)
#include "coreir_interpret.h"
#endif

#if defined(WITH_CLOCKWORK)
#include "rdai_api.h"
#include "clockwork_sim_platform.h"
#include "scalar_reduction_clockwork.h"
#endif

using namespace Halide::Tools;
using namespace Halide::Runtime;

void saveHalideBufferToRawBigEndian(const Halide::Runtime::Buffer<int16_t> &buffer, const std::string &filename) {
    std::ofstream out(filename, std::ios::binary);
    if (!out.is_open()) {
        std::cerr << "Failed to open file for writing: " << filename << std::endl;
        return;
    }

    // Iterate through each element of the buffer and write in big-endian order
    for (int i = 0; i < buffer.number_of_elements(); ++i) {
        int16_t val = buffer(i);
        // Swap bytes if the system is little-endian
        int16_t big_endian_val = (val << 8) | (val >> 8);  // Swap bytes to convert to big-endian
        out.write(reinterpret_cast<const char *>(&big_endian_val), sizeof(big_endian_val));
    }

    out.close();
}

int main(int argc, char ** argv) {
    std::map < std::string, std:: function < void() >> functions;
    ManyInOneOut_ProcessController < int16_t > processor("scalar_reduction", {
        "input.mat",
        "kernel.mat"
    });

    #if defined(WITH_CPU)
    auto cpu_process = [ & ](auto & proc) {
        scalar_reduction(proc.inputs["input.mat"], proc.output);
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
            scalar_reduction_clockwork(proc.inputs["input.mat"], proc.output);
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

    auto vec_height_env = getenv("vec_height");
    auto vec_width_env = getenv("vec_width");

    auto vec_height = vec_height_env ? atoi(vec_height_env) : 2;
    auto vec_width = vec_width_env ? atoi(vec_width_env) : 80;

    std::cout << "using inputs set within process.cpp" << std::endl;
    processor.inputs_preset = true;


    ///// INPUT IMAGE /////
    processor.inputs["input.mat"] = Buffer < int16_t > (vec_width, vec_height);
    auto input_copy_stencil = processor.inputs["input.mat"];
    int max_rand = 7;
    int val = 1;
    for (int y = 0; y < input_copy_stencil.dim(1).extent(); y++) {
        for (int x = 0; x < input_copy_stencil.dim(0).extent(); x++) {
            // input_copy_stencil(x, y) = (rand() % max_rand);
            input_copy_stencil(x, y) = val;
            val++;
        }
    }

    // Output Image
    processor.output = Buffer < int16_t > (vec_height);
    Buffer<int16_t> real_output(1);
    int sum = 0;
    for (int y = 0; y < processor.inputs["input.mat"].dim(1).extent(); y++) {
        for (int x = 0; x < processor.inputs["input.mat"].dim(0).extent(); x++) {
            sum += processor.inputs["input.mat"](x, y);
            sum = int16_t(sum);
        }
    }
    real_output(0) = sum;

    saveHalideBufferToRawBigEndian(processor.inputs["input.mat"], "bin/input_host_stencil.raw");
    saveHalideBufferToRawBigEndian(real_output, "bin/hw_output.raw");

    auto output = processor.process_command(argc, argv);


    return output;
}