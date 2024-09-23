#include <iostream>
#include <math.h>
#include <cstdio>
#include <fstream>
#include <vector>
#include "hardware_process_helper.h"
#include "halide_image_io.h"
#include "coreir.h"

#if defined(WITH_CPU)
   #include "bias_skip_fp.h"
#endif

#if defined(WITH_COREIR)
    #include "coreir_interpret.h"
#endif

#if defined(WITH_CLOCKWORK)
    #include "rdai_api.h"
    #include "clockwork_sim_platform.h"
    #include "bias_skip_fp_clockwork.h"
#endif

using namespace Halide::Tools;
using namespace Halide::Runtime;

nlohmann::json output_starting_json;

union {
  uint32_t val;
  float f;
} union_var;

uint16_t round_to_even_process(float a) {
  //uint32_t e = reinterpret_cast<uint32_t&>(a);
  union_var.f = a;
  uint32_t e = union_var.val;
  
  // round float to even, comment out this codeblock for truncation
  uint32_t half = 0x00008000;
  uint32_t sum = e + half;
  
  // check if bottom bits are all zero
  uint32_t mantissa_mask = 0x0000ffff;
  bool is_zeroed = (sum & mantissa_mask) == 0;
  
  // clear last bit (round even) on tie
  uint32_t clear_mask = ~( ((uint32_t)is_zeroed) << 16);
  e = sum & clear_mask;

  // clear bottom bits
  e = e >> 16;

  //return bfloat16_t::make_from_bits(float_to_bfloat16( expf(bfloat16_to_float(a.to_bits())) ));
  //return bfloat16_t::make_from_bits( (uint16_t)e );
  return (uint16_t)e;
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
    uint16_t bits[2] = {0, b};
    float ret;
    memcpy(&ret, bits, sizeof(float));
    return ret;
}

void saveHalideBufferToRawBigEndian(const Halide::Runtime::Buffer<uint16_t>& buffer, const std::string& filename) {
    std::ofstream out(filename, std::ios::binary);
    if (!out.is_open()) {
        std::cerr << "Failed to open file for writing: " << filename << std::endl;
        return;
    }

    // Iterate through each element of the buffer and write in big-endian order
    for (int i = 0; i < buffer.number_of_elements(); ++i) {
        uint16_t val = buffer(i);
        // Swap bytes if the system is little-endian
        uint16_t big_endian_val = (val << 8) | (val >> 8); // Swap bytes to convert to big-endian
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

    // Get the extents of each dimension in the buffer
    std::vector<int> extents(buffer.dimensions());
    for (int i = 0; i < buffer.dimensions(); i++) {
        extents[i] = buffer.dim(i).extent();
    }

    // Initialize indices to zero for all dimensions
    std::vector<int> indices(buffer.dimensions(), 0);

    // Function to recursively fill the buffer, with column-major order
    std::function<void(int)> fillBuffer = [&](int dim) {
        if (dim == -1) { // All dimensions are set
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
            // Perform byte swap if necessary (for big-endian files)
            value = (value >> 8) | (value << 8);
            buffer(indices.data()) = value;
        } else { // Set the current dimension and recurse
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

std::vector<int> parse_glb_bank_config_env_var(const std::string& env_var_name) {
    std::vector<int> values;
    const char* env_var_value = std::getenv(env_var_name.c_str());
    
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

int main( int argc, char **argv ) {
  std::map<std::string, std::function<void()>> functions;
  ManyInOneOut_ProcessController<uint16_t> processor("bias_skip_fp", {"hw_input_stencil.mat", "hw_bias_stencil.raw", "hw_skip_stencil.mat"});

  #if defined(WITH_CPU)
      auto cpu_process = [&]( auto &proc ) {
        bias_skip_fp(proc.inputs["hw_input_stencil.mat"], proc.inputs["hw_bias_stencil.raw"], proc.inputs["hw_skip_stencil.mat"], proc.output);
      };
      functions["cpu"] = [&](){ cpu_process( processor ); } ;
  #endif
  
  #if defined(WITH_COREIR)
      auto coreir_process = [&]( auto &proc ) {
          run_coreir_on_interpreter<>( "bin/design_top.json",
                                       proc.inputs["hw_input_stencil.mat"], proc.output,
                                       "self.in_arg_0_0_0", "self.out_0_0" );
      };
      functions["coreir"] = [&](){ coreir_process( processor ); };
  #endif
  
  #if defined(WITH_CLOCKWORK)
      auto clockwork_process = [&]( auto &proc ) {
        RDAI_Platform *rdai_platform = RDAI_register_platform( &rdai_clockwork_sim_ops );
        if ( rdai_platform ) {
          printf( "[RUN_INFO] found an RDAI platform\n" );
          bias_skip_fp_clockwork(proc.inputs["hw_input_stencil.mat"], proc.inputs["hw_bias_stencil.raw"], proc.inputs["hw_skip_stencil.mat"], proc.output);
          RDAI_unregister_platform( rdai_platform );
        } else {
          printf("[RUN_INFO] failed to register RDAI platform!\n");
        }
      };
      functions["clockwork"] = [&](){ clockwork_process( processor ); };
  #endif

  auto OX = getenv("out_img");
  auto OC = getenv("n_oc");
  auto PO_L = getenv("pad_o_left");
  auto PO_R = getenv("pad_o_right");
  auto TRUNC_SIZE = getenv("trunc_size");

  auto out_img = OX ? atoi(OX) : 56;
  auto n_oc = OC ? atoi(OC) : 32;
  int pad_o_left = PO_L ? atoi(PO_L) : 0;
  int pad_o_right = PO_R ? atoi(PO_R) : 0;
  int trunc_size = TRUNC_SIZE ? atoi(TRUNC_SIZE) : 0;

  int imgsize_x = out_img;
  int imgsize_y = out_img;
  int imgsize_x_padded = imgsize_x + pad_o_left + pad_o_right;
  int imgsize_y_padded = imgsize_y + pad_o_left + pad_o_right;
      
  // Add all defined functions
  processor.run_calls = functions;

  processor.inputs["hw_input_stencil.mat"]        = Buffer<uint16_t>(n_oc, imgsize_x, imgsize_y);
  processor.inputs["hw_bias_stencil.raw" ]        = Buffer<uint16_t>(n_oc, imgsize_x, imgsize_y);
  processor.inputs["hw_skip_stencil.mat" ]        = Buffer<uint16_t>(n_oc, imgsize_x, imgsize_y);
  processor.output                                = Buffer<uint16_t>(n_oc, imgsize_x, imgsize_y);
  auto output_gold_tensor = Buffer<uint16_t>(n_oc, imgsize_x_padded, imgsize_y_padded);
  output_gold_tensor.fill(0);

  processor.inputs_preset = true;
  
  for (int y = 0; y < processor.inputs["hw_input_stencil.mat"].dim(2).extent(); y++) {
    for (int x = 0; x < processor.inputs["hw_input_stencil.mat"].dim(1).extent(); x++) {
      for (int w = 0; w < processor.inputs["hw_input_stencil.mat"].dim(0).extent(); w++) {
        processor.inputs["hw_input_stencil.mat"](w, x, y) = float_to_bfloat16_process(
          // [-7, 7]
          ((float)rand() / RAND_MAX) * 14.0 - 7.0
        );
      }
    }
  }

  for (int w = 0; w < n_oc; w++) {
    for (int x = 0; x < imgsize_x; x++) {
      for (int y = 0; y < imgsize_y; y++) {
        processor.inputs["hw_bias_stencil.raw"](w, x, y) = float_to_bfloat16_process(
          // [-7, 7]
          ((float)rand() / RAND_MAX) * 14.0 - 7.0
        );
      }
    }
  }

  for (int w = 0; w < n_oc; w++) {
    for (int x = 0; x < imgsize_x; x++) {
      for (int y = 0; y < imgsize_y; y++) {
        processor.inputs["hw_skip_stencil.mat"](w, x, y) = float_to_bfloat16_process(
          // [-7, 7]
          ((float)rand() / RAND_MAX) * 14.0 - 7.0
        );
      }
    }
  }

  // Gold output
  for (int w = 0; w < n_oc; w++) {
    for (int x_padded = 0; x_padded < imgsize_x_padded; x_padded++) {
      for (int y_padded = 0; y_padded < imgsize_y_padded; y_padded++) {
        // Check if the current position is within the truncation area
        if (x_padded >= imgsize_x_padded - trunc_size || y_padded >= imgsize_y_padded - trunc_size) {
          // Set output to zero
          output_gold_tensor(w, x_padded, y_padded) = float_to_bfloat16_process(0.0f);
        } else if (x_padded < pad_o_left || x_padded >= pad_o_left + imgsize_x ||
                   y_padded < pad_o_left || y_padded >= pad_o_left + imgsize_y) {
          // Positions in padding area, set them to zero
          output_gold_tensor(w, x_padded, y_padded) = float_to_bfloat16_process(0.0f);
        } else {
          int x = x_padded - pad_o_left;
          int y = y_padded - pad_o_left;
          float input_val = bfloat16_to_float_process(processor.inputs["hw_input_stencil.mat"](w, x, y));
          float bias_val = bfloat16_to_float_process(processor.inputs["hw_bias_stencil.raw"](w, x, y));
          float skip_val = bfloat16_to_float_process(processor.inputs["hw_skip_stencil.mat"](w, x, y));
          float sum = input_val + bias_val + skip_val;
          float result = sum;
          output_gold_tensor(w, x_padded, y_padded) = float_to_bfloat16_process(result);
        }
      }
    }
  }

  std::cout << "Writing hw_input_stencil.mat to bin folder" << std::endl;
  save_image(processor.inputs["hw_input_stencil.mat"], "bin/hw_input_stencil.mat");

  std::cout << "Writing hw_bias_stencil.raw to bin folder" << std::endl;
  saveHalideBufferToRawBigEndian(processor.inputs["hw_bias_stencil.raw"], "bin/hw_bias_stencil.raw");

  std::cout << "Writing hw_skip_stencil.mat to bin folder" << std::endl;
  save_image(processor.inputs["hw_skip_stencil.mat"], "bin/hw_skip_stencil.mat");
  
  std::cout << "Writing output to bin folder" << std::endl;
  save_image(output_gold_tensor, "bin/hw_output.mat");

  // Generate glb_bank_config.json if "USE_GLB_BANK_CONFIG" is 1
  std::cout << "Checking for GLB bank configuration..." << std::endl;
  std::cout << "USE_GLB_BANK_CONFIG = " << getenv("USE_GLB_BANK_CONFIG") << std::endl;
  if (getenv("USE_GLB_BANK_CONFIG") && std::stoi(getenv("USE_GLB_BANK_CONFIG"))) {
    std::vector<int> hw_input_stencil = parse_glb_bank_config_env_var("HW_INPUT_STENCIL_POS");
    std::vector<int> hw_bias_stencil = parse_glb_bank_config_env_var("HW_BIAS_STENCIL_POS");
    std::vector<int> hw_skip_stencil = parse_glb_bank_config_env_var("HW_SKIP_STENCIL_POS");
    std::vector<int> hw_output_stencil = parse_glb_bank_config_env_var("HW_OUTPUT_STENCIL_POS");
    std::vector<int> glb_inputs = parse_glb_bank_config_env_var("GLB_INPUTS");

    // Create the glb_bank_config.json structure
    json config = {
        {"inputs", {
            {"hw_input_stencil", hw_input_stencil},
            {"hw_bias_stencil", hw_bias_stencil},
            {"hw_skip_stencil", hw_skip_stencil}
        }},
        {"outputs", {
            {"hw_output_stencil", hw_output_stencil}
        }},
        {"glb_inputs", glb_inputs}
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
  
  return processor.process_command(argc, argv);
  
}
