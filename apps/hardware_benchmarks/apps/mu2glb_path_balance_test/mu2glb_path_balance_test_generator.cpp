#include "Halide.h"

namespace {

using namespace std;

using namespace Halide;

int inImgSize = 64;
int outImgSize = inImgSize;

class PathBalancing : public Halide::Generator<PathBalancing> {
public:
    Input<Buffer<uint8_t>>  input{"input", 2};
    Output<Buffer<uint8_t>> output{"output", 2};

    GeneratorParam<int32_t> myunroll{"myunroll", 1};
    GeneratorParam<int32_t> myunroll_E64{"myunroll_E64", 4};
    GeneratorParam<int32_t> myunroll_E64_MB{"myunroll_E64_MB", 8};

    void generate() {
        /* THE ALGORITHM */

        Var x("x"), y("y");

        Func input_copy, mu_hw_input, hw_output;
        Func mult("mult");
        //input_copy(x, y) = cast<uint16_t>(input(x, y));
        //mu_hw_input(x, y) = input_copy(x, y);
        mu_hw_input(x, y) = cast<uint16_t>(input(x, y));

        mult(x, y) = mu_hw_input(x,y) * 2;
        hw_output(x, y) = mult(x, y);
        output(x, y) = cast<uint8_t>(hw_output(x, y));

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR) ||
            get_target().has_feature(Target::HLS)) {
          Var xi,yi, xo,yo;

          //mu_hw_input.compute_root();
          hw_output.compute_root();
          hw_output.tile(x,y, xo,yo, xi,yi, outImgSize, outImgSize)
            .hw_accelerate(xi, xo);
          //hw_output.tile(x,y, xo,yo, xi,yi, 64, 64-2)
            //.hw_accelerate(xi, xo);
          output.bound(x, 0, outImgSize);
          output.bound(y, 0, outImgSize);

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

          output.bound(x, 0, outImgSize);
          output.bound(y, 0, outImgSize);

          //mu_hw_input.compute_root();
          hw_output.compute_root();
          hw_output
              .tile(x,y, xo,yo, xi,yi, outImgSize, outImgSize)
              .hw_accelerate(xi, xo);
          hw_output.unroll(xi, unroll);

          mult.compute_at(hw_output, xo)
            .unroll(x, unroll);

          //mu_hw_input.store_at(hw_output, xo).compute_at(hw_output, xo);
          mu_hw_input.stream_to_accelerator();
          mu_hw_input.in().unroll(x, unroll);
          //input_copy.compute_root();

        } else {
          mult.compute_root();
        }
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(PathBalancing, mu2glb_path_balance_test)
