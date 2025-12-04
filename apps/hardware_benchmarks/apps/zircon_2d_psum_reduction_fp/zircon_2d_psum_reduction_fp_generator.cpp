#include "Halide.h"

namespace {

using namespace std;

using namespace Halide;


// Partial sum reduction
class PSUM_reduction : public Halide::Generator<PSUM_reduction> {
public:
    Input<Buffer<uint16_t>>  input{"input", 2};
    Input<Buffer<uint16_t>>  partial_sum_input{"partial_sum_input", 2};
    Output<Buffer<uint16_t>> output{"output", 2};

    GeneratorParam<int> seq_len{"seq_len", 64};
    GeneratorParam<int> hidden_dim{"hidden_dim", 128};

    GeneratorParam<int32_t> myunroll{"myunroll", 1};
    GeneratorParam<int32_t> myunroll_E64{"myunroll_E64", 16};
    GeneratorParam<int32_t> myunroll_E64_MB{"myunroll_E64_MB", 16};


    void generate() {
        /* THE ALGORITHM */

        Var d("d"), n("n");

        Func mu_hw_input("mu_hw_input");
        Func hw_partial_sum_input("hw_partial_sum_input");
        Func hw_output("hw_output");
        Func result;

        mu_hw_input(d, n) = cast<bfloat16_t>(input(d, n));
        hw_partial_sum_input(d, n) = cast<bfloat16_t>(partial_sum_input(d, n));
        result(d, n) = mu_hw_input(d, n) + hw_partial_sum_input(d, n);

        hw_output(d, n) = result(d, n);
        output(d, n) = cast<uint16_t>(hw_output(d, n));

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR) ||
            get_target().has_feature(Target::HLS)) {
          Var d_glb,n_glb, d_host,n_host;

          //mu_hw_input.compute_root();
          hw_output.compute_root();
          hw_output.tile(d,n, d_host,n_host, d_glb,n_glb, hidden_dim, seq_len)
            .hw_accelerate(d_glb, d_host);
          output.bound(d, 0, hidden_dim);
          output.bound(n, 0, seq_len);

          mu_hw_input.store_at(hw_output, d_host).compute_at(hw_output, d_glb);

          mu_hw_input.stream_to_accelerator();

          cout << "Loop nest" << endl;
          hw_output.print_loop_nest();

        } else if (get_target().has_feature(Target::Clockwork)) {
          Var d_glb,n_glb, d_host,n_host;

          const char* e64_mode_env = getenv("E64_MODE_ON");
          const char* e64_multi_bank_mode_env = getenv("E64_MULTI_BANK_MODE_ON");
          int unroll = myunroll;
          if (e64_multi_bank_mode_env && std::stoi(e64_multi_bank_mode_env) == 1) {
            unroll = myunroll_E64_MB;
          } else if (e64_mode_env && std::stoi(e64_mode_env) == 1) {
            unroll = myunroll_E64;
          }

          output.bound(d, 0, hidden_dim);
          output.bound(n, 0, seq_len);

          //mu_hw_input.compute_root();
          hw_output.compute_root();
          hw_output
              .tile(d,n, d_host,n_host, d_glb,n_glb, hidden_dim, seq_len)
              .hw_accelerate(d_glb, d_host);
          hw_output.unroll(d_glb, unroll);

          result.compute_at(hw_output, d_host)
            .unroll(d, unroll);

          //mu_hw_input.store_at(hw_output, d_host).compute_at(hw_output, d_host);
          mu_hw_input.stream_to_accelerator();
          mu_hw_input.in().unroll(d, unroll);
          //input_copy.compute_root();

          hw_partial_sum_input.stream_to_accelerator();
          hw_partial_sum_input.in().unroll(d, unroll);

        } else {
          result.compute_root();
        }
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(PSUM_reduction, zircon_2d_psum_reduction_fp)
