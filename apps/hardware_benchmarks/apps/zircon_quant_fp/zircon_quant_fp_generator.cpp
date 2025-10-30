#include "Halide.h"

namespace {

using namespace std;

using namespace Halide;
using namespace Halide::ConciseCasts;

auto QUANT_SCALE = getenv("QUANT_SCALE");
const float quant_scale = QUANT_SCALE ? atof(QUANT_SCALE) : 0.5f;

// Quantize
// Compute pipeline: quantize (bf16 mul) -> bf16-to-int16 typecast -> data packing
class Quantize : public Halide::Generator<Quantize> {
public:
    Input<Buffer<uint16_t>>  input{"input", 3};
    Output<Buffer<uint16_t>> output{"output", 3};

    GeneratorParam<int> out_img{"out_img", 14};
    GeneratorParam<int> n_oc{"n_oc", 64};

    GeneratorParam<int32_t> myunroll{"myunroll", 1};
    GeneratorParam<int32_t> myunroll_E64{"myunroll_E64", 16};
    GeneratorParam<int32_t> myunroll_E64_MB{"myunroll_E64_MB", 32};

    void generate() {
        /* THE ALGORITHM */

      Var x("x"), y("y"), c("c"), c_pack("c_pack");
      Func hw_input("hw_input");
      Func hw_output("hw_output");
      Func unpacked_result, result;

      hw_input(c, x, y) = cast<bfloat16_t>(input(c, x, y));

      // Quantize and typecast
      unpacked_result(c, x, y) = e8m0_quant(hw_input(c, x, y) * cast<bfloat16_t>(quant_scale), reinterpret(type_of<bfloat16_t>(), cast<uint16_t>(127)));

      // Pack
      result(c_pack, x, y) = bit8_pack(unpacked_result(2 * c_pack + 1, x, y), unpacked_result(2 * c_pack, x, y));

      hw_output(c_pack, x, y) = result(c_pack, x, y);
      output(c_pack, x, y) = cast<uint16_t>(hw_output(c_pack, x, y));

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
          output.bound(c_pack, 0, int(n_oc / 2));

          hw_output.compute_root();
          hw_output
            .tile(x,y, xo,yo, xi,yi, out_img, out_img)
            .reorder(c_pack, xi, yi, xo, yo)
            .hw_accelerate(xi, xo);
          hw_output.unroll(c_pack, std::max(1,int(unroll / 2)));

          result.compute_at(hw_output, xo).unroll(c_pack, std::max(1,int(unroll / 2)));

          hw_input.stream_to_accelerator();
          hw_input.in().unroll(c, unroll);

        } else {

        }
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(Quantize, zircon_quant_fp)
