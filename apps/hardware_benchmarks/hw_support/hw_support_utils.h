#ifndef HW_SUPPORT_UTILS_H
#define HW_SUPPORT_UTILS_H

#include <cstdint>
#include <string>
#include <stdexcept>
#include <functional>
#include <vector>
#include "HalideBuffer.h"
using Halide::Runtime::Buffer;

// Rounds a float to bfloat16 using round-to-even
uint16_t round_to_even_process(float a);

// Converts a float to bfloat16
uint16_t float_to_bfloat16_process(float f);

// Converts a bfloat16 (stored as uint16_t) back to float
float bfloat16_to_float_process(uint16_t b);

// Save a Halide buffer (of uint16_t values) to a file in raw big-endian format
void saveHalideBufferToRawBigEndian(const Halide::Runtime::Buffer<uint16_t>& buffer, const std::string& filename);

// Load raw data from a big-endian file into a Halide buffer
void loadRawDataToBuffer(const std::string& filename, Halide::Runtime::Buffer<uint16_t>& buffer);

// Copy a file from source path to destination path
void copyFile(const std::string &srcPath, const std::string &dstPath);

// Check if a file exists
bool file_exists(const std::string& name);

extern "C" {
    // CPU implementation that packs two bf16 values (stored as uint16_t) into one uint16_t.
    uint16_t bf16toint8_pack(uint16_t in0, uint16_t in1);
}

extern "C" {
    // CPU implementation that packs two 16-bit data's lower 8 bits into one 16-bit data
    uint16_t bit8_pack(uint16_t in0, uint16_t in1);
}

extern "C" {
    uint16_t abs_max(uint16_t in0, uint16_t in1);
}

extern "C" {
    // CPU implementations for 8-bit unpack operations
    uint16_t bit8_unpack_high(uint16_t in0);
    uint16_t bit8_unpack_low(uint16_t in0);
}

#endif // HW_SUPPORT_UTILS_H
