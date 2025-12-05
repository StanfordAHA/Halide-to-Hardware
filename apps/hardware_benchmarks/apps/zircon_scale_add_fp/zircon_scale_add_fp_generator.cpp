#include "Halide.h"

namespace {

using namespace std;

using namespace Halide;


auto ATTN_SCALE = getenv("ATTN_SCALE");
const float attn_scale = ATTN_SCALE ? atof(ATTN_SCALE) : 0.5f; // TODO: MUST TAKE RECIPROCAL!

// Attention scaling (const fpmul) + attention masking (fpadd)
class scale_add : public Halide::Generator<scale_add> {
public:
    Input<Buffer<uint16_t>>  input{"input", 2};
    Input<Buffer<uint16_t>>  attn_mask_input{"attn_mask_input", 2};
    Output<Buffer<uint16_t>> output{"output", 2};

    GeneratorParam<int> seq_len{"seq_len", 64};

    GeneratorParam<int32_t> myunroll{"myunroll", 4};
    GeneratorParam<int32_t> myunroll_E64{"myunroll_E64", 16};
    GeneratorParam<int32_t> myunroll_E64_MB{"myunroll_E64_MB", 16};

    void generate() {
        /* THE ALGORITHM */

        Var x("x"), y("y");

        Func mu_hw_input("mu_hw_input");
        Func hw_attn_mask_input("hw_attn_mask_input");
        Func hw_output("hw_output");
        Func result("result");

        mu_hw_input(x, y) = cast<bfloat16_t>(input(x, y));
        hw_attn_mask_input(x, y) = cast<bfloat16_t>(attn_mask_input(x, y));

        result(x, y) = (mu_hw_input(x, y) * cast<bfloat16_t>(attn_scale)) + hw_attn_mask_input(x, y);
        hw_output(x, y) = result(x, y);
        output(x, y) = cast<uint16_t>(hw_output(x, y));

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR) ||
            get_target().has_feature(Target::HLS)) {
          Var xi,yi, xo,yo;

          //mu_hw_input.compute_root();
          hw_output.compute_root();
          hw_output.tile(x,y, xo,yo, xi,yi, seq_len, seq_len)
            .hw_accelerate(xi, xo);
          //hw_output.tile(x,y, xo,yo, xi,yi, 64, 64-2)
            //.hw_accelerate(xi, xo);
          output.bound(x, 0, seq_len);
          output.bound(y, 0, seq_len);

          mu_hw_input.store_at(hw_output, xo).compute_at(hw_output, xi);

          mu_hw_input.stream_to_accelerator();

          cout << "Loop nest" << endl;
          hw_output.print_loop_nest();

        } else if (get_target().has_feature(Target::Clockwork)) {
          Var xi,yi, xo,yo;

          const char* e64_mode_env = getenv("E64_MODE_ON");
          const char* e64_multi_bank_mode_env = getenv("E64_MULTI_BANK_MODE_ON");
          int unroll = myunroll;
          if (e64_multi_bank_mode_env && std::stoi(e64_multi_bank_mode_env) == 1) {
            unroll = myunroll_E64_MB;
          } else if (e64_mode_env && std::stoi(e64_mode_env) == 1) {
            unroll = myunroll_E64;
          }

          output.bound(x, 0, seq_len);
          output.bound(y, 0, seq_len);

          //mu_hw_input.compute_root();
          hw_output.compute_root();
          hw_output
              .tile(x,y, xo,yo, xi,yi, seq_len, seq_len)
              .hw_accelerate(xi, xo);
          hw_output.unroll(xi, unroll);

          result.compute_at(hw_output, xo)
            .unroll(x, unroll);

          //mu_hw_input.store_at(hw_output, xo).compute_at(hw_output, xo);
          mu_hw_input.stream_to_accelerator();
          mu_hw_input.in().unroll(x, unroll);
          //input_copy.compute_root();


          hw_attn_mask_input.stream_to_accelerator();
          hw_attn_mask_input.in().unroll(x, unroll);

        } else {
          result.compute_root();
        }
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(scale_add, zircon_scale_add_fp)
