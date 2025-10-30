#include "Halide.h"

namespace {

using namespace std;

using namespace Halide;
using namespace Halide::ConciseCasts;


auto DEQUANT_SCALE = getenv("DEQUANT_SCALE");
const float dequant_scale = DEQUANT_SCALE ? atof(DEQUANT_SCALE) : 0.5f;

// DequantizeResidualRelu
// Compute pipeline: bf16 mul (dequantize) -> residual -> relu
class DequantizeResidualRelu : public Halide::Generator<DequantizeResidualRelu> {
public:
    Input<Buffer<uint16_t>>  input{"input", 3};
    Input<Buffer<uint16_t>>  residual_input{"residual_input", 3};
    Output<Buffer<uint16_t>> output{"output", 3};

    GeneratorParam<int> out_img{"out_img", 14};
    GeneratorParam<int> n_oc{"n_oc", 64};

    GeneratorParam<int32_t> myunroll{"myunroll", 1};
    GeneratorParam<int32_t> myunroll_E64{"myunroll_E64", 16};
    GeneratorParam<int32_t> myunroll_E64_MB{"myunroll_E64_MB", 32};

    void generate() {
        /* THE ALGORITHM */

      Var x("x"), y("y"), c("c");
      Func mu_hw_input("mu_hw_input");
      Func hw_residual_input("hw_residual_input");
      Func hw_output("hw_output");
      Func result;

      mu_hw_input(c, x, y) = cast<bfloat16_t>(input(c, x, y));
      hw_residual_input(c, x, y) = cast<bfloat16_t>(residual_input(c, x, y));

      // dequantize (fpmul), add residual, relu
      result(c, x, y) = max(mu_hw_input(c, x, y) * cast<bfloat16_t>(dequant_scale) + hw_residual_input(c, x, y), cast<bfloat16_t>(0.0f));

      hw_output(c, x, y) = result(c, x, y);
      output(c, x, y) = cast<uint16_t>(hw_output(c, x, y));

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR) ||
            get_target().has_feature(Target::HLS)) {

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

          output.bound(x, 0, out_img);
          output.bound(y, 0, out_img);
          output.bound(c, 0, n_oc);

          hw_output.compute_root();
          hw_output
            .tile(x,y, xo,yo, xi,yi, out_img, out_img)
            .reorder(c, xi, yi, xo, yo)
            .hw_accelerate(xi, xo);
          hw_output.unroll(c, unroll);

          result.compute_at(hw_output, xo).unroll(c, unroll);

          mu_hw_input.stream_to_accelerator();
          mu_hw_input.in().unroll(c, unroll);

          hw_residual_input.stream_to_accelerator();
          hw_residual_input.in().unroll(c, unroll);

        } else {

        }
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(DequantizeResidualRelu, zircon_deq_ResReLU_fp)