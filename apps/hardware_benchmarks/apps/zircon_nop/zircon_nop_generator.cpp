#include "Halide.h"

namespace {

using namespace std;

using namespace Halide;

class ZirconNOP : public Halide::Generator<ZirconNOP> {
public:
    Input<Buffer<uint8_t>>  input{"input", 3};
    Output<Buffer<uint8_t>> output{"output", 3};

    GeneratorParam<int> out_img{"out_img", 14};
    GeneratorParam<int> n_oc{"n_oc", 256};

    GeneratorParam<int32_t> myunroll{"myunroll", 1};
    GeneratorParam<int32_t> myunroll_E64{"myunroll_E64", 16};
    GeneratorParam<int32_t> myunroll_E64_MB{"myunroll_E64_MB", 16};

    void generate() {
        /* THE ALGORITHM */
        Var x("x"), y("y"), c("c");

        Func input_copy, mu_hw_input, hw_output;
        Func nop("nop");
        mu_hw_input(c, x, y) = cast<uint16_t>(input(c, x, y));

        nop(c, x, y) = mu_hw_input(c, x, y);
        hw_output(c, x, y) = nop(c, x, y);
        output(c, x, y) = cast<uint8_t>(hw_output(c, x, y));

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR) ||
            get_target().has_feature(Target::HLS)) {
          Var xi,yi, xo,yo;

          //mu_hw_input.compute_root();
          hw_output.compute_root();
          hw_output.tile(x,y, xo,yo, xi,yi, out_img, out_img)
            .reorder(c, xi, yi, xo, yo)
            .hw_accelerate(xi, xo);
          output.bound(x, 0, out_img);
          output.bound(y, 0, out_img);
          output.bound(c, 0, n_oc);

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

          output.bound(x, 0, out_img);
          output.bound(y, 0, out_img);
          output.bound(c, 0, n_oc);

          hw_output.compute_root();
          hw_output
              .tile(x,y, xo,yo, xi,yi, out_img, out_img)
              .reorder(c, xi, yi, xo, yo)
              .hw_accelerate(xi, xo);
          hw_output.unroll(c, unroll);

          nop.compute_at(hw_output, xo)
            .unroll(c, unroll);

          mu_hw_input.stream_to_accelerator();
          mu_hw_input.in().unroll(c, unroll);

        } else {
          nop.compute_root();
        }
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(ZirconNOP, zircon_nop)
