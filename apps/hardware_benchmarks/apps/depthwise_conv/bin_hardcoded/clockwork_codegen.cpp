#include "cgra_flow.h"
#include "depthwise_conv_compute.h"
#include "depthwise_conv_memory.cpp"

int main(int argc, char **argv) {
  std::vector<prog> prgs = { depthwise_conv() };
  std::vector<std::string> args(argv + 1, argv + argc);
  for (auto& prg : prgs) {
    size_t i=0;
    while (i < args.size()) {
      if (args[i] == "opt") {
        generate_optimized_code(prg);
      } else if (args[i] == "unopt") {
        generate_unoptimized_code(prg);
      } else if (args[i] == "compile_mem") {
        preprocess_prog(prg);

        // Run Frontend Test, generate gold TB
        auto cpu = unoptimized_result(prg);

        // Run Memory Mapper and dump collateral into dir
        string dir = "./map_result";
        compile_app_for_garnet_single_port_mem(prg, dir, true, true, false);

      } else if (args[i] == "compile_and_test_mem") {
        preprocess_prog(prg);

        // Run Frontend Test, generate gold TB
        auto cpu = unoptimized_result(prg);

        // Run Memory Mapper and dump collateral into dir
        string dir = "./map_result";
        compile_app_for_garnet_single_port_mem(prg, dir, /*gen_config_only=*/false, /*enable_ponds=*/true, /*use_metamapper*/false);

        // Run interconnect agnostic tb
        auto cgra = cgra_flow_result(prg, dir);

        sanity_check(prg, cpu, cgra);
      } else if (args[i] == "compile_mem_use_metamapper") {
        preprocess_prog(prg);

        // Run Frontend Test, generate gold TB
        auto cpu = unoptimized_result(prg);

        // Run Memory Mapper and dump collateral into dir
        string dir = "./map_result";
        compile_app_for_garnet_single_port_mem(prg, dir, true, true, true);

      } else if (args[i] == "compile_and_test_mem_use_metamapper") {
        preprocess_prog(prg);

        // Run Frontend Test, generate gold TB
        auto cpu = unoptimized_result(prg);

        // Run Memory Mapper and dump collateral into dir
        string dir = "./map_result";
        compile_app_for_garnet_single_port_mem(prg, dir, /*gen_config_only=*/false, /*enable_ponds=*/true, /*use_metamapper*/true);

        // Run interconnect agnostic tb
        auto cgra = cgra_flow_result(prg, dir);

        sanity_check(prg, cpu, cgra);
      }
      i += 1;
    }
  }
  return 0;
}
