#include "hw_support_utils.h"

#include <fstream>
#include <iostream>
#include <cstring>
#include <functional>
#include <vector>
#include <stdexcept>

// Include Halide Buffer header
#include "HalideBuffer.h"

// Function to round a float to bfloat16 using round-to-even.
uint16_t round_to_even_process(float a) {
    // Use a local union for bit-level manipulation.
    union {
        uint32_t val;
        float f;
    } union_var;
    union_var.f = a;
    uint32_t e = union_var.val;

    // round float to even: add half (0x00008000)
    uint32_t half = 0x00008000;
    uint32_t sum = e + half;

    // check if bottom bits are all zero
    uint32_t mantissa_mask = 0x0000ffff;
    bool is_zeroed = (sum & mantissa_mask) == 0;

    // clear last bit (round even) on tie
    uint32_t clear_mask = ~( ((uint32_t)is_zeroed) << 16);
    e = sum & clear_mask;

    // clear bottom bits and shift right by 16 bits
    e = e >> 16;

    return static_cast<uint16_t>(e);
}

uint16_t float_to_bfloat16_process(float f) {
    // The conversion process uses round-to-even.
    return round_to_even_process(f);
}

float bfloat16_to_float_process(uint16_t b) {
    // Assume little-endian floats
    uint16_t bits[2] = {0, b};
    float ret;
    std::memcpy(&ret, bits, sizeof(float));
    return ret;
}

void saveHalideBufferToRawBigEndian(const Halide::Runtime::Buffer<uint16_t>& buffer, const std::string& filename) {
    std::ofstream out(filename, std::ios::binary);
    if (!out.is_open()) {
        std::cerr << "Failed to open file for writing: " << filename << std::endl;
        return;
    }

    // Iterate through each element of the buffer and write in big-endian order.
    for (int i = 0; i < buffer.number_of_elements(); ++i) {
        uint16_t val = buffer(i);
        // Swap bytes if the system is little-endian.
        uint16_t big_endian_val = (val << 8) | (val >> 8);
        out.write(reinterpret_cast<const char*>(&big_endian_val), sizeof(big_endian_val));
    }

    out.close();
}

void loadRawDataToBuffer(const std::string& filename, Halide::Runtime::Buffer<uint16_t>& buffer) {
    std::ifstream inFile(filename, std::ios::binary);
    if (!inFile) {
        std::cerr << "Failed to open file for reading: " << filename << std::endl;
        return;
    }

    // Get the extents of each dimension in the buffer.
    std::vector<int> extents(buffer.dimensions());
    for (int i = 0; i < buffer.dimensions(); i++) {
        extents[i] = buffer.dim(i).extent();
    }

    // Initialize indices to zero for all dimensions.
    std::vector<int> indices(buffer.dimensions(), 0);

    // Recursively fill the buffer in column-major order.
    std::function<void(int)> fillBuffer = [&](int dim) {
        if (dim == -1) { // All dimensions are set.
            uint16_t value;
            inFile.read(reinterpret_cast<char*>(&value), sizeof(value));
            if (!inFile) {
                std::cerr << "Error reading data for indices ";
                for (int i = 0; i < indices.size(); ++i) {
                    std::cerr << indices[i] << (i < indices.size() - 1 ? ", " : "");
                }
                std::cerr << std::endl;
                throw std::runtime_error("Failed to read data from file");
            }
            // Swap bytes for big-endian to little-endian conversion.
            value = (value >> 8) | (value << 8);
            buffer(indices.data()) = value;
        } else {
            // Recurse for the current dimension.
            for (int i = 0; i < extents[dim]; ++i) {
                indices[dim] = i;
                fillBuffer(dim - 1);
            }
            indices[dim] = 0;
        }
    };

    try {
        fillBuffer(buffer.dimensions() - 1);
    } catch (const std::exception& e) {
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

bool file_exists(const std::string& name) {
    std::ifstream f(name.c_str());
    return f.good();
}

extern "C" {
    uint16_t bf16toint8_pack(uint16_t in0, uint16_t in1) {
        // Process first bf16 value (in0 to int8 conversion)
        uint16_t a = in0;
        uint16_t sign_a = a & 0x8000;
        uint16_t mant_a = (a & 0x007F) | 0x0080;
        uint16_t exp_a = (a & 0x7F80) >> 7;
        int biased_exp_a = static_cast<int>(exp_a);
        int unbiased_exp_a = biased_exp_a - 127;
        uint32_t mant_shift_a = (unbiased_exp_a < 0) ? 0 : ((uint32_t)mant_a << unbiased_exp_a);
        uint8_t unsigned_res_a = static_cast<uint8_t>((mant_shift_a >> 7) & 0xFF);
        int8_t int8_a = (sign_a == 0x8000) ? -static_cast<int8_t>(unsigned_res_a) : static_cast<int8_t>(unsigned_res_a);

        // Process second bf16 value (in1 to int8 conversion)
        uint16_t b = in1;
        uint16_t sign_b = b & 0x8000;
        uint16_t mant_b = (b & 0x007F) | 0x0080;
        uint16_t exp_b = (b & 0x7F80) >> 7;
        int biased_exp_b = static_cast<int>(exp_b);
        int unbiased_exp_b = biased_exp_b - 127;
        uint32_t mant_shift_b = (unbiased_exp_b < 0) ? 0 : ((uint32_t)mant_b << unbiased_exp_b);
        uint8_t unsigned_res_b = static_cast<uint8_t>((mant_shift_b >> 7) & 0xFF);
        int8_t int8_b = (sign_b == 0x8000) ? -static_cast<int8_t>(unsigned_res_b) : static_cast<int8_t>(unsigned_res_b);

        // Pack the two int8 values into one 16-bit value.
        uint16_t packed = (static_cast<uint16_t>(static_cast<uint8_t>(int8_a)) << 8) | (static_cast<uint8_t>(int8_b));
        return packed;
    }
}
