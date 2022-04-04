#include "Halide.h"

namespace {

using namespace std;

using namespace Halide;
using namespace Halide::ConciseCasts;

int inImgSize = 64;
int outImgSize = inImgSize;

class PointwiseMultiplication : public Halide::Generator<PointwiseMultiplication> {
public:
    Input<Buffer<uint8_t>>  input{"input", 2};
    Output<Buffer<uint8_t>> output{"output", 2};

    GeneratorParam<int32_t> myunroll{"myunroll", 1};
  
    Expr mul1(Expr a, Expr b) {
      return i16(( (i32(a)) * (i32(b)) ) >> 8);
    }

    void generate() {
        /* THE ALGORITHM */

        Var x("x"), y("y");

        Func input_copy, hw_input, hw_output;
        Func mult("mult");
        //input_copy(x, y) = cast<uint16_t>(input(x, y));
        //hw_input(x, y) = input_copy(x, y);
        hw_input(x, y) = cast<uint16_t>(input(x, y));
        
        mult(x, y) = mul1(hw_input(x,y), 65433);
        hw_output(x, y) = mult(x, y);
        output(x, y) = cast<uint8_t>(hw_output(x, y));

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR) ||
            get_target().has_feature(Target::HLS)) {
          Var xi,yi, xo,yo;
          
          //hw_input.compute_root();
          hw_output.compute_root();
          hw_output.tile(x,y, xo,yo, xi,yi, outImgSize, outImgSize)
            .hw_accelerate(xi, xo);
          //hw_output.tile(x,y, xo,yo, xi,yi, 64, 64-2)
            //.hw_accelerate(xi, xo);
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
          
          //hw_input.compute_root();
          hw_output.compute_root();
          hw_output
              .tile(x,y, xo,yo, xi,yi, outImgSize, outImgSize)
              .hw_accelerate(xi, xo);
          hw_output.unroll(xi, myunroll);

          mult.compute_at(hw_output, xo)
            .unroll(x, myunroll);
          
          //hw_input.store_at(hw_output, xo).compute_at(hw_output, xo);
          hw_input.stream_to_accelerator();
          hw_input.in().unroll(x, myunroll);
          //input_copy.compute_root();
          
        } else {
          mult.compute_root();
        }
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(PointwiseMultiplication, pointwise)
