#include "Halide.h"

namespace {

using namespace std;

using namespace Halide;
using namespace Halide::ConciseCasts;


int inImgSize = 448;
int outImgSize = inImgSize;

// Residual additon fused with ReLU
class ResidualRelu : public Halide::Generator<ResidualRelu> {
public:
    Input<Buffer<uint16_t>>  input{"input", 2};
    Input<Buffer<uint16_t>>  residual_input{"residual_input", 2};
    Output<Buffer<uint16_t>> output{"output", 2};

    GeneratorParam<int32_t> myunroll{"myunroll", 1};
    GeneratorParam<int32_t> myunroll_E64{"myunroll_E64", 16};
    GeneratorParam<int32_t> myunroll_E64_MB{"myunroll_E64_MB", 32};

    void generate() {
        /* THE ALGORITHM */

      Var x("x"), y("y");
      Func mu_hw_input("mu_hw_input");
      Func hw_residual_input("hw_residual_input");
      Func hw_output("hw_output");
      Func result;

      mu_hw_input(x, y) = cast<bfloat16_t>(input(x, y));
      hw_residual_input(x, y) = cast<bfloat16_t>(residual_input(x, y));

      result(x, y) = max(mu_hw_input(x, y) + hw_residual_input(x, y), cast<bfloat16_t>(0.0f));

      hw_output(x, y) = result(x, y);
      output(x, y) = cast<uint16_t>(hw_output(x, y));

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

          output.bound(x, 0, outImgSize);
          output.bound(y, 0, outImgSize);

          hw_output.compute_root();
          hw_output
            .tile(x,y, xo,yo, xi,yi, outImgSize, outImgSize)
            .reorder(xi, yi, xo, yo)
            .hw_accelerate(xi, xo);
          hw_output.unroll(xi, unroll);

          result.compute_at(hw_output, xo).unroll(x, unroll);

          mu_hw_input.stream_to_accelerator();
          mu_hw_input.in().unroll(x, unroll);

          hw_residual_input.stream_to_accelerator();
          hw_residual_input.in().unroll(x, unroll);

        } else {

        }
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(ResidualRelu, residual_relu_fp)
