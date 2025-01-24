#include "coreir.h"
#include "halide_image_io.h"
#include "hardware_process_helper.h"
#include <cstdio>
#include <fstream>
#include <iostream>
#include <math.h>
#include <vector>

#if defined(WITH_CPU)
#include "softmax.h"
#endif

#if defined(WITH_COREIR)
#include "coreir_interpret.h"
#endif

#if defined(WITH_CLOCKWORK)
#include "clockwork_sim_platform.h"
#include "rdai_api.h"
#include "softmax_clockwork.h"
#endif

using namespace Halide::Tools;
using namespace Halide::Runtime;

nlohmann::json output_starting_json;

union {
    uint32_t val;
    float f;
} union_var;

uint16_t round_to_even_process(float a) {
    // uint32_t e = reinterpret_cast<uint32_t&>(a);
    union_var.f = a;
    uint32_t e = union_var.val;

    // round float to even, comment out this codeblock for truncation
    uint32_t half = 0x00008000;
    uint32_t sum = e + half;

    // check if bottom bits are all zero
    uint32_t mantissa_mask = 0x0000ffff;
    bool is_zeroed = (sum & mantissa_mask) == 0;

    // clear last bit (round even) on tie
    uint32_t clear_mask = ~(((uint32_t) is_zeroed) << 16);
    e = sum & clear_mask;

    // clear bottom bits
    e = e >> 16;

    // return bfloat16_t::make_from_bits(float_to_bfloat16( expf(bfloat16_to_float(a.to_bits())) ));
    // return bfloat16_t::make_from_bits( (uint16_t)e );
    return (uint16_t) e;
}

// Similar routines for bfloat. It's somewhat simpler.
uint16_t float_to_bfloat16_process(float f) {
    //    uint16_t ret[2];
    //    memcpy(ret, &f, sizeof(float));
    //    // Assume little-endian floats
    //    return ret[1];
    return round_to_even_process(f);
}

float bfloat16_to_float_process(uint16_t b) {
    // Assume little-endian floats
    uint16_t bits[2] = { 0, b };
    float ret;
    memcpy(&ret, bits, sizeof(float));
    return ret;
}

void saveHalideBufferToRawBigEndian(const Halide::Runtime::Buffer<uint16_t> &buffer, const std::string &filename) {
    std::ofstream out(filename, std::ios::binary);
    if (!out.is_open()) {
        std::cerr << "Failed to open file for writing: " << filename << std::endl;
        return;
    }

    // Iterate through each element of the buffer and write in big-endian order
    for (int i = 0; i < buffer.number_of_elements(); ++i) {
        uint16_t val = buffer(i);
        // Swap bytes if the system is little-endian
        uint16_t big_endian_val = (val << 8) | (val >> 8);  // Swap bytes to convert to big-endian
        out.write(reinterpret_cast<const char *>(&big_endian_val), sizeof(big_endian_val));
    }

    out.close();
}

void loadRawDataToBuffer(const std::string &filename, Halide::Runtime::Buffer<uint16_t> &buffer) {
    std::ifstream inFile(filename, std::ios::binary);
    if (!inFile) {
        std::cerr << "Failed to open file for reading: " << filename << std::endl;
        return;
    }

    // Get the extents of each dimension in the buffer
    std::vector<int> extents(buffer.dimensions());
    for (int i = 0; i < buffer.dimensions(); i++) {
        extents[i] = buffer.dim(i).extent();
    }

    // Initialize indices to zero for all dimensions
    std::vector<int> indices(buffer.dimensions(), 0);

    // Function to recursively fill the buffer, with column-major order
    std::function<void(int)> fillBuffer = [&](int dim) {
        if (dim == -1) {  // All dimensions are set
            uint16_t value;
            inFile.read(reinterpret_cast<char *>(&value), sizeof(value));
            if (!inFile) {
                std::cerr << "Error reading data for indices ";
                for (int i = 0; i < indices.size(); ++i) {
                    std::cerr << indices[i] << (i < indices.size() - 1 ? ", " : "");
                }
                std::cerr << std::endl;
                throw std::runtime_error("Failed to read data from file");
            }
            // Perform byte swap if necessary (for big-endian files)
            value = (value >> 8) | (value << 8);
            buffer(indices.data()) = value;
        } else {  // Set the current dimension and recurse
            for (int i = 0; i < extents[dim]; ++i) {
                indices[dim] = i;
                fillBuffer(dim - 1);
            }
            indices[dim] = 0;
        }
    };

    try {
        // From the last dimension down to 0; reverse order for column-major)
        fillBuffer(buffer.dimensions() - 1);
    } catch (const std::exception &e) {
        std::cerr << "An exception occurred: " << e.what() << std::endl;
        inFile.close();
        return;
    }

    inFile.close();
}

void copyFile(const std::string &srcPath, const std::string &dstPath) {
    std::ifstream src(srcPath, std::ios::binary);
    std::ofstream dst(dstPath, std::ios::binary);

    if (!src.is_open() || !dst.is_open()) {
        throw std::runtime_error("Error opening files while copying from " + srcPath + " to " + dstPath);
    }

    dst << src.rdbuf();
}

bool file_exists(const std::string &name) {
    std::ifstream f(name.c_str());
    return f.good();
}

std::vector<int> parse_glb_bank_config_env_var(const std::string &env_var_name) {
    std::vector<int> values;
    const char *env_var_value = std::getenv(env_var_name.c_str());

    if (env_var_value) {
        std::string value_str = env_var_value;
        std::istringstream iss(value_str);
        std::string token;

        // Split the string by commas and convert to integers
        while (std::getline(iss, token, ',')) {
            // Trim potential whitespace
            token.erase(0, token.find_first_not_of(" \t\n\r\f\v"));
            token.erase(token.find_last_not_of(" \t\n\r\f\v") + 1);
            values.push_back(std::stoi(token));
        }
    } else {
        std::cerr << "Environment variable " << env_var_name << " not found." << std::endl;
    }

    return values;
}

int main(int argc, char **argv) {
    std::map<std::string, std::function<void()>> functions;
    OneInOneOut_ProcessController<uint16_t> processor("softmax");

#if defined(WITH_CPU)
    auto cpu_process = [&](auto &proc) {
        softmax(proc.input, proc.output);
    };
    functions["cpu"] = [&]() { cpu_process(processor); };
#endif

#if defined(WITH_COREIR)
    auto coreir_process = [&](auto &proc) {
        run_coreir_on_interpreter<>("bin/design_top.json",
                                    proc.input, proc.output,
                                    "self.in_arg_0_0_0", "self.out_0_0");
    };
    functions["coreir"] = [&]() { coreir_process(processor); };
#endif

#if defined(WITH_CLOCKWORK)
    auto clockwork_process = [&](auto &proc) {
        RDAI_Platform *rdai_platform = RDAI_register_platform(&rdai_clockwork_sim_ops);
        if (rdai_platform) {
            printf("[RUN_INFO] found an RDAI platform\n");
            softmax_clockwork(proc.input, proc.output);
            RDAI_unregister_platform(rdai_platform);
        } else {
            printf("[RUN_INFO] failed to register RDAI platform!\n");
        }
    };
    functions["clockwork"] = [&]() { clockwork_process(processor); };
#endif

    auto VEC_LEN = getenv("vec_len");
    auto vec_len = VEC_LEN ? atoi(VEC_LEN) : 32;

    // Add all defined functions
    processor.run_calls = functions;

    processor.input = Buffer<uint16_t>(vec_len);
    processor.output = Buffer<uint16_t>(1);

    processor.inputs_preset = true;

    // Input
    for (int x = 0; x < processor.input.dim(0).extent(); x++) {
        processor.input(x) = float_to_bfloat16_process(
            ((float) rand() / RAND_MAX) * 14.0 - 7.0);
    }

    // Gold output
    float sumVal = 0.0f;
    for (int x = 0; x < processor.input.dim(0).extent(); x++) {
        float val = bfloat16_to_float_process(processor.input(x));
        sumVal += val;
    }
    for (int z = 0; z < processor.output.dim(0).extent(); z++) {
        processor.output(z) = float_to_bfloat16_process(sumVal);
    }

    std::cout << "Writing hw_input_stencil.raw to bin folder" << std::endl;
    saveHalideBufferToRawBigEndian(processor.input, "bin/hw_input_stencil.raw");

    std::cout << "Writing hw_output.raw to bin folder" << std::endl;
    saveHalideBufferToRawBigEndian(processor.output, "bin/hw_output.raw");

    return processor.process_command(argc, argv);
}
