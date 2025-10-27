#include "hardware_process_helper.h"
#include "halide_image_io.h"
#include "hw_support_utils.h"

#if defined(WITH_CPU)
   #include "maxpooling_dense_rv_fp.h"
#endif

#if defined(WITH_COREIR)
    #include "coreir_interpret.h"
#endif

#if defined(WITH_CLOCKWORK)
    #include "rdai_api.h"
    #include "clockwork_sim_platform.h"
    #include "maxpooling_dense_rv_fp_clockwork.h"
#endif

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main( int argc, char **argv ) {
  std::map<std::string, std::function<void()>> functions;
  ManyInOneOut_ProcessController<uint16_t> processor("maxpooling_dense_rv_fp", {"input_host_stencil.mat"});

  #if defined(WITH_CPU)
      auto cpu_process = [&]( auto &proc ) {
        maxpooling_dense_rv_fp(proc.inputs["input_host_stencil.mat"], proc.output);
      };
      functions["cpu"] = [&](){ cpu_process( processor ); } ;
  #endif

  #if defined(WITH_COREIR)
      auto coreir_process = [&]( auto &proc ) {
          run_coreir_on_interpreter<>( "bin/design_top.json",
                                       proc.inputs["input_host_stencil.mat"], proc.output,
                                       "self.in_arg_0_0_0", "self.out_0_0" );
      };
      functions["coreir"] = [&](){ coreir_process( processor ); };
  #endif

  #if defined(WITH_CLOCKWORK)
      auto clockwork_process = [&]( auto &proc ) {
        RDAI_Platform *rdai_platform = RDAI_register_platform( &rdai_clockwork_sim_ops );
        if ( rdai_platform ) {
          printf( "[RUN_INFO] found an RDAI platform\n" );
          maxpooling_dense_rv_fp_clockwork(proc.inputs["input_host_stencil.mat"], proc.output);
          RDAI_unregister_platform( rdai_platform );
        } else {
          printf("[RUN_INFO] failed to register RDAI platform!\n");
        }
      };
      functions["clockwork"] = [&](){ clockwork_process( processor ); };
  #endif

    // Add all defined functions
    processor.run_calls = functions;

    // Set enviroment variable to set these:
    //  HALIDE_GEN_ARGS="in_img_fake=28 pad=1 ksize=3 stride=1 n_ic_fake=8 k_oc=8"

    auto in_img_env = getenv("in_img");
    auto ksize_env = getenv("ksize");
    auto stride_env = getenv("stride");
    auto unroll_env = getenv("unroll");
    auto n_ic_env = getenv("n_ic");

    auto in_img = in_img_env ? atoi(in_img_env) : 112;
    auto ksize = ksize_env ? atoi(ksize_env) : 3;
    auto stride = stride_env ? atoi(stride_env) : 2;
    auto unroll = unroll_env ? atoi(unroll_env) : 16;
    auto n_ic = n_ic_env ? atoi(n_ic_env) : 8;

    auto OX = getenv("in_img_fake");
    auto IC = getenv("n_ic_fake");

    auto in_img_fake = OX ? atoi(OX) : 57;
    auto n_ic_fake = IC ? atoi(IC) : 56;

    int X = in_img_fake;
    int Y = in_img_fake;
    int K_X = ksize;
    int K_Y = K_X;
    int C = n_ic_fake;

    // input image processor
    processor.inputs["input_host_stencil.mat"] = Halide::Runtime::Buffer<uint16_t>(C, X, Y);
    processor.inputs_preset = true;
    auto real_input = Buffer<uint16_t>(n_ic, in_img, in_img);
    int i=1; (void) i;
    int sparsity = 0;
    for (int y = 0; y < real_input.dim(2).extent(); y++) {
        for (int x = 0; x < real_input.dim(1).extent(); x++) {
            for (int c = 0; c < real_input.dim(0).extent(); c++) {
                if (rand() % 100 < sparsity) {
                    real_input(c, x, y) = float_to_bfloat16_process(0.0f);
                } else {
                    int random_int = (rand() % 1024) - 512;
                    real_input(c, x, y) = float_to_bfloat16_process(static_cast<float>(random_int));
                }
            }
        }
    }

    std::cout << "real input has dims: " << real_input.dim(0).extent() << "x"
              << real_input.dim(1).extent() << "x"
              << real_input.dim(2).extent() << "\n";

    // output image processor
    int imgsize_x = std::floor( (X - K_X) / stride ) + 1;
    int imgsize_y = std::floor( (Y - K_Y) / stride ) + 1;
    processor.output = Halide::Runtime::Buffer<uint16_t>(C, imgsize_x, imgsize_y);

    // Real gold output
    int real_out_x = std::floor( (in_img - ksize) / stride ) + 1;
    int real_out_y = std::floor( (in_img - ksize) / stride ) + 1;
    auto real_output = Buffer<uint16_t>(n_ic, real_out_x, real_out_y);

    // Max pooling operation
    for (int c = 0; c < real_output.dim(0).extent(); c++) { // For each channel
        for (int x = 0; x < real_output.dim(1).extent(); x++) {
            for (int y = 0; y < real_output.dim(2).extent(); y++) {
                float max_val = -std::numeric_limits<float>::infinity();
                for (int kx = 0; kx < ksize; kx++) { // Kernel height
                    for (int ky = 0; ky < ksize; ky++) { // Kernel width
                        int src_x = x * stride + kx;
                        int src_y = y * stride + ky;
                        if (src_x < in_img && src_y < in_img) {
                            float val = bfloat16_to_float_process(real_input(c, src_x, src_y));
                            max_val = std::max(max_val, val); // Keep the max value
                        }
                    }
                }
                real_output(c, x, y) = float_to_bfloat16_process(max_val);
            }
        }
    }

    std::cout << "real output has dims: " << real_output.dim(0).extent() << "x"
              << real_output.dim(1).extent() << "x"
              << real_output.dim(2).extent() << "\n";

    // Check for gold tensors
    bool use_resnet_gold = std::filesystem::exists("resnet18_gold_tensors/input_host_stencil_0_0_0_0.raw");
    if (use_resnet_gold) {
        std::cout << "Using ResNet18 gold tensors" << std::endl;
        // Copy gold tensors to bin directory
        std::filesystem::copy_file(
            "resnet18_gold_tensors/input_host_stencil_0_0_0_0.raw",
            "bin/input_host_stencil.raw",
            std::filesystem::copy_options::overwrite_existing
        );
        std::filesystem::copy_file(
            "resnet18_gold_tensors/hw_output_0_0_0_0.raw",
            "bin/hw_output.raw",
            std::filesystem::copy_options::overwrite_existing
        );
    }
    else {
        std::cout << "Generating random tensors" << std::endl;
        save_halide_buffer_to_raw(real_input, "bin/input_host_stencil.raw");
        save_halide_buffer_to_raw(real_output, "bin/hw_output.raw");
    }

    // Create glb bank config
    using namespace glb_cfg;
    // inputs, outputs, mu_inputs
    const config_spec spec = {
        {tensor_spec{"input_host_stencil", {"x_coord"}}},
        {tensor_spec{"hw_output", {"x_coord"}}},
        {}
    };
    write_glb_bank_config(spec);

    return processor.process_command(argc, argv);
}
