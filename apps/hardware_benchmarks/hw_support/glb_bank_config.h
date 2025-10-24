#pragma once
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include "coreir.h"

// Parse a GLB bank configuration to extract vector of integers from environment variable
std::vector<int> parse_glb_bank_config_num_list(const std::string &env_var_name);

namespace glb_cfg {

// Check if the environment variable is set and is not 0
inline bool env_flag(const char *var_name) {
    const char *v = std::getenv(var_name);
    if (!v) return false;
    try {
        return std::stoi(v) != 0;
    } catch (...) {
        return false;
    }
}

// Convert tensor name to environment variable base name: "input_host_stencil" -> "INPUT_HOST_STENCIL"
inline std::string to_env_base(const std::string &tensor_name) {
    std::string s;
    s.reserve(tensor_name.size());
    for (char ch : tensor_name) {
        if (std::isalnum(static_cast<unsigned char>(ch)))
            s.push_back(std::toupper(ch));
        else
            s.push_back('_');
    }
    return s;
}

// Map a logical attribute to its env suffix: x_coord, E64_packed, use_multi_bank_mode, and more if needed
inline std::string attr_to_suffix(const std::string &attr) {
    if (attr == "x_coord") return "POS";
    if (attr == "E64_packed") return "PACKED";
    if (attr == "use_multi_bank_mode") return "MULTI_BANK_MODE";
    if (attr == "bank_toggle_mode") return "BANK_TOGGLE_MODE";
    std::string up;
    up.reserve(attr.size());
    for (char ch : attr)
        up.push_back(std::toupper(ch));
    return up;
}

// Read the list of values for a given attribute from the environment variable
inline std::vector<int> read_attr_list(const std::string &tensor_name,
                                       const std::string &attr) {
    const std::string base = to_env_base(tensor_name);
    const std::string env = base + "_" + attr_to_suffix(attr);
    return parse_glb_bank_config_num_list(env);
}

struct tensor_spec {
    std::string name;  // e.g., "matrix_host_stencil"
    std::vector<std::string> attrs;  // e.g., {"x_coord","E64_packed","use_multi_bank_mode"}
};

struct config_spec {
    std::vector<tensor_spec> inputs;
    std::vector<tensor_spec> outputs;
    std::vector<tensor_spec> mu_inputs;
};

// Build the json object from a spec
inline json build_json_from_env(const config_spec& spec) {
    json config = json::object();

    auto build_side = [&](const std::vector<tensor_spec>& tensors) {
        json side = json::array();
        for (const auto& t : tensors) {
            json tensor_obj;
            for (const auto& attr : t.attrs) {
                tensor_obj[attr] = read_attr_list(t.name, attr);
            }
            json entry;
            entry[t.name] = tensor_obj;
            side.push_back(entry);
        }
        return side;
    };

    if (!spec.inputs.empty())    config["inputs"]    = build_side(spec.inputs);
    if (!spec.outputs.empty())   config["outputs"]   = build_side(spec.outputs);
    if (!spec.mu_inputs.empty()) config["mu_inputs"] = build_side(spec.mu_inputs);

    return config;
}

inline bool write_json(const json &config, const std::string &path) {
    std::ofstream f(path);
    if (!f.is_open()) return false;
    f << config.dump(4) << '\n';
    return true;
}

// Helper called from each process.cpp
inline int write_glb_bank_config(const config_spec &spec,
                                 const std::string &out_path = "bin/glb_bank_config.json",
                                 const char *env_var_name = "USE_GLB_BANK_CONFIG") {
    std::cout << "Checking for GLB bank configuration..." << std::endl;
    std::cout << env_var_name << " = " << std::getenv(env_var_name) << std::endl;

    if (!env_flag(env_var_name)) return 0;

    try {
        json config = build_json_from_env(spec);
        if (!write_json(config, out_path)) {
            std::cerr << "Unable to open file for writing: " << out_path << std::endl;
            return 1;
        }
        std::cout << "Successfully wrote to " << out_path << std::endl;
    } catch (const std::exception &e) {
        std::cerr << "Error while generating GLB bank config: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}

}  // namespace glb_cfg
