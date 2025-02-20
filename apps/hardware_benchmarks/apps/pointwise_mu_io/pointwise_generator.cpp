#include "Halide.h"

namespace {

using namespace std;

using namespace Halide;

int inImgSize = 64;
int outImgSize = inImgSize;

class PointwiseMultiplication : public Halide::Generator<PointwiseMultiplication> {
public:
    Input<Buffer<uint8_t>>  input{"input", 2};
    Output<Buffer<uint8_t>> output{"output", 2};

    GeneratorParam<int32_t> myunroll{"myunroll", 1};
  
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

          output.bound(x, 0, outImgSize);
          output.bound(y, 0, outImgSize);
          
          //mu_hw_input.compute_root();
          hw_output.compute_root();
          hw_output
              .tile(x,y, xo,yo, xi,yi, outImgSize, outImgSize)
              .hw_accelerate(xi, xo);
          hw_output.unroll(xi, myunroll);

          mult.compute_at(hw_output, xo)
            .unroll(x, myunroll);
          
          //mu_hw_input.store_at(hw_output, xo).compute_at(hw_output, xo);
          mu_hw_input.stream_to_accelerator();
          mu_hw_input.in().unroll(x, myunroll);
          //input_copy.compute_root();
          
        } else {
          mult.compute_root();
        }
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(PointwiseMultiplication, pointwise)
