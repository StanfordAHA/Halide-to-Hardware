#include "Halide.h"

namespace {

using namespace Halide;

class SequentialDivides : public Halide::Generator<SequentialDivides> {
public:
    Input<Buffer<uint8_t>>  input{"input", 2};
    Output<Buffer<uint8_t>> output{"output", 2};

  int ksize = 3;
  int imgsize = 62;

    void generate() {
        /* THE ALGORITHM */

        Var x("x"), y("y");

        Func hw_input("hw_input");
        Func hw_input_copy("hw_input_copy");
        hw_input(x, y) = cast<uint16_t>(input(x, y));
        hw_input_copy(x, y) = hw_input(x, y);

        Func div1, div2, div3, div4;
        div1(x, y) = hw_input_copy(x, y) / 7;
        div2(x, y) = div1(x, y) / 7;
        div3(x, y) = div2(x, y) / 7;
        div4(x, y) = div3(x, y) / 7;

        Func hw_output("hw_output");
        hw_output(x, y) = cast<uint8_t>(div4(x, y));
        output(x, y) = hw_output(x,y);


        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR) ||
            get_target().has_feature(Target::HLS)) {
          Var xi,yi, xo,yo;

          output.bound(x, 0, imgsize);
          output.bound(y, 0, imgsize);

          hw_output.compute_root();

          hw_output.tile(x,y, xo,yo, xi,yi, imgsize, imgsize)
            .hw_accelerate(xi, xo);

          hw_input.compute_at(hw_output, xi).store_at(hw_output, xo);
          hw_input.stream_to_accelerator();
          
        } else if (get_target().has_feature(Target::Clockwork)) {
          Var xi,yi, xo,yo;

          output.bound(x, 0, imgsize);
          output.bound(y, 0, imgsize);

          hw_output.compute_root();

          hw_output.tile(x,y, xo,yo, xi,yi, imgsize, imgsize)
            .hw_accelerate(xi, xo);

          div4.compute_at(hw_output, xo);

          div4.compute_share_root(x);
          div3.compute_share(div4);
          div2.compute_share(div4);
          div1.compute_share(div4);
          
          hw_input_copy.compute_at(hw_output, xo);
          hw_input.stream_to_accelerator();
          hw_input.compute_root();
            
        } else {  // schedule to CPU
          hw_output.compute_root();
        }

    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(SequentialDivides, divs)
