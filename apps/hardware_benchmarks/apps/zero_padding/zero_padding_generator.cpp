#include "Halide.h"

namespace {

using namespace std;

using namespace Halide;

int pad = 1;
int inImgSize = 64;
int outImgSize = inImgSize + 2 * pad;

class ZeroPadding : public Halide::Generator<ZeroPadding> {
public:
    Input<Buffer<uint8_t>>  input{"input", 2};
    Output<Buffer<uint8_t>> output{"output", 2};

    GeneratorParam<int32_t> myunroll{"myunroll", 1};
  
    void generate() {
        /* THE ALGORITHM */

        Var x("x"), y("y");

        Func input_copy, hw_input, hw_output;
        hw_input(x, y) = cast<uint16_t>(input(x, y));

        Func hw_input_bounded;
        hw_input_bounded = BoundaryConditions::constant_exterior(hw_input, 0, {{0, inImgSize}, {0, inImgSize}});

        Func padded;
        // V1 Repeated Padding
        // padded(x, y) = hw_input(
        //   clamp(x - pad, 0, inImgSize - 1),
        //   clamp(y - pad, 0, inImgSize - 1)
        // );

        // V2 Repeated Padding
        // padded(x, y) = cast<uint16_t>(0);
        // padded(x, y) += hw_input(
        //   clamp(x - pad, 0, inImgSize - 1),
        //   clamp(y - pad, 0, inImgSize - 1)
        // );

        // V3 Zero Padding
        // padded(x, y) = select(
        //   x-pad >= 0 && x-pad <= inImgSize-1 && y-pad >=0 && y-pad <= inImgSize-1,
        //   hw_input(clamp(x - pad, 0, inImgSize - 1), clamp(y - pad, 0, inImgSize - 1)),
        //   cast<uint16_t>(0)
        // );

        // V4 Zero Padding
        // padded(x, y) = cast<uint16_t>(0);
        // padded(x, y) += select(
        //   x >= pad && x <= inImgSize + pad - 1 && y >= pad && y <= inImgSize + pad -1,
        //   // hw_input(clamp(x - pad, 0, inImgSize - 1), clamp(y - pad, 0, inImgSize - 1)),
        //   // hw_input(x - pad, y - pad),
        //   hw_input_bounded(x - pad, y - pad),
        //   cast<uint16_t>(0)
        // );

        // V5 Zero Padding
        padded(x, y) = select(
          x >= pad && x <= inImgSize + pad - 1 && y >= pad && y <= inImgSize + pad -1,
          hw_input_bounded(x - pad, y - pad),
          cast<uint16_t>(0)
        );

        hw_output(x, y) = padded(x, y);
        output(x, y) = cast<uint8_t>(hw_output(x, y));

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR) ||
            get_target().has_feature(Target::HLS)) {
          Var xi,yi, xo,yo;

          hw_output.compute_root();
          hw_output.tile(x,y, xo,yo, xi,yi, outImgSize, outImgSize)
            .hw_accelerate(xi, xo);
          output.bound(x, 0, outImgSize);
          output.bound(y, 0, outImgSize);

          hw_input.store_at(hw_output, xo).compute_at(hw_output, xi);

          hw_input.stream_to_accelerator();

          cout << "Loop nest" << endl;
          hw_output.print_loop_nest();

        } else if (get_target().has_feature(Target::Clockwork)) {
          Var xi,yi, xo,yo;

          output.bound(x, 0, outImgSize);
          output.bound(y, 0, outImgSize);
          
          hw_output.compute_root();
          hw_output
              .tile(x,y, xo,yo, xi,yi, outImgSize, outImgSize)
              .hw_accelerate(xi, xo);

          padded.compute_at(hw_output, xo).unroll(x, myunroll);
          
          hw_input.stream_to_accelerator();
          hw_input.in().unroll(x, myunroll);
          
        } else {
          padded.compute_root();
        }
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(ZeroPadding, zero_padding)
