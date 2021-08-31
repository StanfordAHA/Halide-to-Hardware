/*
 * An application for applying a Gaussian blur.
 * It uses a 3x3 stencil with constant weights.
 */

#include "Halide.h"
#include <experimental/random>

namespace {

using namespace Halide;

// Size of blur for gradients.
const int blockSize = 3;

class GEMM : public Halide::Generator<GEMM> {
public:
    Input<Buffer<uint8_t>>  inputA{"inputA", 2};
    //Input<Buffer<uint8_t>>  inputB{"inputB", 2};
    Output<Buffer<uint8_t>> output{"output", 2};

    int size = 64;
  
    void generate() {
        /* THE ALGORITHM */

        Var x("x"), y("y");

        Func hw_inputA("hw_inputA"), hw_inputB("hw_inputB");
        hw_inputA(x,y) = cast<uint16_t>(inputA(x,y));
        hw_inputB(x,y) = cast<uint16_t>(0);
        //hw_inputB(x,y) = cast<uint16_t>(std::experimental::randint(0,65535));
	for (int i = 0; i < size; i++) {
            hw_inputB(i,0) = cast<uint16_t>(std::experimental::randint(0,65535));
        }

        Func prod("prod");
        RDom r(0, size);
        prod(x,y) = 0;
        prod(x,y) += hw_inputA(x, r) * hw_inputB(r, y);

        Func hw_output("hw_output");
        hw_output(x,y) = prod(x,y);
        output(x,y) = cast<uint8_t>(hw_output(x,y));

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR)) {
        } else if (get_target().has_feature(Target::Clockwork)) {
          Var xi,yi, xo,yo;

          output.bound(x, 0, 1);
          output.bound(y, 0, 1);

          hw_output.compute_root();
          hw_output.tile(x,y, xo,yo, xi,yi, 1, 1)
            .hw_accelerate(xi, xo);
          hw_inputB.compute_at(hw_output, xo);
//          hw_inputA.compute_at(hw_output, xo);
          prod.update()
            .unroll(r.x, size);
          prod.compute_at(hw_output, xo);
          hw_inputA.stream_to_accelerator();
//          hw_inputB.compute_root();
          //hw_inputB.stream_to_accelerator();
        } else {    // schedule to CPU
        }
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(GEMM, gemm)

