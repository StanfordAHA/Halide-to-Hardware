#include "Halide.h"

namespace {

using namespace std;

using namespace Halide;


class ZirconNOP : public Halide::Generator<ZirconNOP> {
public:
    Input<Buffer<uint8_t>>  input{"input", 2};
    Output<Buffer<uint8_t>> output{"output", 2};

    GeneratorParam<int> seq_len{"seq_len", 64};
    GeneratorParam<int> hidden_dim{"hidden_dim", 128};

    GeneratorParam<int32_t> myunroll{"myunroll", 1};
    GeneratorParam<int32_t> myunroll_E64{"myunroll_E64", 16};
    GeneratorParam<int32_t> myunroll_E64_MB{"myunroll_E64_MB", 16};


    void generate() {
        /* THE ALGORITHM */

        Var d("d"), n("n");

        Func input_copy, hw_input, hw_output;
        Func nop("nop");
        hw_input(d, n) = cast<uint16_t>(input(d, n));

        nop(d, n) = hw_input(d,n);
        hw_output(d, n) = nop(d, n);
        output(d, n) = cast<uint8_t>(hw_output(d, n));

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR) ||
            get_target().has_feature(Target::HLS)) {
          Var d_glb,n_glb, d_host,n_host;

          //hw_input.compute_root();
          hw_output.compute_root();
          hw_output.tile(d,n, d_host,n_host, d_glb,n_glb, hidden_dim, seq_len)
            .hw_accelerate(d_glb, d_host);
          output.bound(d, 0, hidden_dim);
          output.bound(n, 0, seq_len);

          hw_input.store_at(hw_output, d_host).compute_at(hw_output, d_glb);

          hw_input.stream_to_accelerator();

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

          //hw_input.compute_root();
          hw_output.compute_root();
          hw_output
              .tile(d,n, d_host,n_host, d_glb,n_glb, hidden_dim, seq_len)
              .hw_accelerate(d_glb, d_host);
          hw_output.unroll(d_glb, unroll);

          nop.compute_at(hw_output, d_host)
            .unroll(d, unroll);

          //hw_input.store_at(hw_output, d_host).compute_at(hw_output, d_host);
          hw_input.stream_to_accelerator();
          hw_input.in().unroll(d, unroll);
          //input_copy.compute_root();

        } else {
          nop.compute_root();
        }
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(ZirconNOP, nop_2d)
