#ifndef HW_SUPPORT_UTILS_H
#define HW_SUPPORT_UTILS_H

#include <cstdint>
#include <string>
#include <stdexcept>
#include <functional>
#include <vector>
#include <filesystem>
#include "glb_bank_config.h"
#include "HalideBuffer.h"
using Halide::Runtime::Buffer;

// Rounds a float to bfloat16 using round-to-even
uint16_t round_to_even_process(float a);

// Converts a float to bfloat16
uint16_t float_to_bfloat16_process(float f);

// Converts a bfloat16 (stored as uint16_t) back to float
float bfloat16_to_float_process(uint16_t b);

// Save a Halide buffer (of uint16_t values) to a file in raw big-endian format
void save_halide_buffer_to_raw(const Halide::Runtime::Buffer<uint16_t>& buffer, const std::string& filename);

// Load raw data from a big-endian file into a Halide buffer
void load_raw_to_halide_buffer(const std::string& filename, Halide::Runtime::Buffer<uint16_t>& buffer);

extern "C" {
    // CPU implementations for custom instructions

    // Converts bf16 to int8 and pack
    uint16_t bf16toint8_pack(uint16_t in0, uint16_t in1);

    // Pack lower 8bits of two 16bit inputs
    uint16_t bit8_pack(uint16_t in0, uint16_t in1);

    // Max of two bf16's absolute values
    uint16_t abs_max(uint16_t in0, uint16_t in1);

    // Unpack high and low 8bits of a 16bit input
    uint16_t bit8_unpack_high(uint16_t in0);
    uint16_t bit8_unpack_low(uint16_t in0);

    // E8M0 biased shared exp calculation from bf16
    uint16_t get_shared_exp(uint16_t in0);

    // E8M0 quantization division
    uint16_t e8m0_quant(uint16_t in0, uint16_t in1);
}

#endif // HW_SUPPORT_UTILS_H
