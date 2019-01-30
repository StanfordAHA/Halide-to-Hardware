#include "Halide.h"

namespace {

using namespace Halide;

class Gemm : public Halide::Generator<Gemm> {
public:
    Input<Buffer<uint8_t>>  input_A{"input_A", 2};
    Input<Buffer<uint8_t>>  input_B{"input_B", 2};

    Func build() {
        /* THE ALGORITHM */
      
        Func output("output");
        Var x("x"), y("y");

        Func f_gemm("gemm");
        RDom r(0, 64);

        f_gemm(x, y) += input_A(r, x) * input_B(r, y);

        output(x, y) = f_gemm(x, y);

        input_A.dim(0).set_bounds(0, 64);
        input_B.dim(0).set_bounds(0, 64);

        /* THE SCHEDULE */

        output.compile_to_lowered_stmt("gemm.html",
               output.infer_arguments(),
               Halide::HTML);
        output.compile_to_c("gemm.cpp",
               output.infer_arguments(),
               "gemm");
        return output;
   }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(Gemm, gemm)

