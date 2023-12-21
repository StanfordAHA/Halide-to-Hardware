#include "Halide.h"

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

struct Tensor {
  Halide::Func f;
  std::vector<int> shape;
  std::string name;
  std::vector<Halide::Func> fs;
};

class ConvolutionKernel : public Halide::Generator<ConvolutionKernel> {
public:
    Input<Buffer<uint8_t>>  inputq{"inputq", 2};
    Input<Buffer<uint8_t>>  inputk{"inputk", 2};
    Input<Buffer<uint8_t>>  inputv{"inputv", 2};
    Output<Buffer<uint8_t>> output{"output", 2};

    int imgsize = 64;

    void generate() {
        /* THE ALGORITHM */
        Var x("x"), y("y"), z("z");

        Func Q("Q"), K("K"), V("V");
        Q(x, y) = u16(inputq(x, y));
        K(x, y) = u16(inputk(x, y));
        V(x, y) = u16(inputv(x, y));

        // matrix multiplication of Q and K^t
        Func mul_QKt("mul_QKt");
        //assert(inputq.dim(0).extent() == inputk.dim(0).extent());
        RDom r_k(0, imgsize); //inputq.dim(0).extent());
        mul_QKt(x, y) = u16(0);
        mul_QKt(x, y) += Q(r_k.x, y) * K(r_k.x, y);

        // scaled by d_k (dim of K)
        Func scaled_QKt("scaled_QKt");
        Expr d_k = imgsize; //inputk.dim(1).extent();
        scaled_QKt(x, y) = mul_QKt(x, y) / d_k;

        // soft max
        Func exp_vals("exp_vals"), soft_QKt("soft_QKt");
        RDom r_soft(0, 10);
        exp_vals(x, y) = exp(scaled_QKt(x, y));
        soft_QKt(x, y) = exp_vals(x, y) / sum( exp_vals(r_soft.x, y) );

        // matrix multiplication of result (QK^t) and V
        Func mul_QKtV("mul_QKtV");
        //assert(inputk.dim(1).extent() == inputv.dim(1).extent());
        RDom r_v(0, imgsize);
        mul_QKtV(x, y) = float(0);
        mul_QKtV(x, y) += soft_QKt(r_v.x, y) * V(x, r_v.x);

        Func hw_output("hw_output");
        hw_output(x, y) = mul_QKtV(x, y);
        output(x, y) = u8(hw_output(x,y));

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR) ||
            get_target().has_feature(Target::HLS)) {
          
        } else if (get_target().has_feature(Target::Clockwork) || get_target().has_feature(Target::Pono)) {
          Var xi,yi, xo,yo;

          output.bound(x, 0, imgsize);
          output.bound(y, 0, imgsize);
          mul_QKt.bound(x, 0, imgsize);
          mul_QKt.bound(y, 0, imgsize);
          K.bound(y, 0, imgsize);
          Q.bound(x, 0, imgsize);

          hw_output.compute_root();

          hw_output
              .tile(x,y, xo,yo, xi,yi, imgsize, imgsize)
              .reorder(xi,yi, xo,yo)
              .hw_accelerate(xi, xo);

          mul_QKtV.compute_at(hw_output, xo);

          soft_QKt.compute_at(hw_output, xo);

          mul_QKt.compute_at(hw_output, xo);
          
          Q.stream_to_accelerator();
          K.stream_to_accelerator();
          V.stream_to_accelerator();

        } else {  // schedule to CPU
          hw_output.compute_root();
        }

    }

  Func softmax_layer(const Tensor &input, const int classes, const std::string &name) {
    Var c("c");
    assert(input.shape[0] == classes);
    RDom r(0, classes);
    Func exp_vals;
    exp_vals(c) = exp(input.f(c));
    Func output(name);
    output(c) = exp_vals(c) / sum(exp_vals(r.x));
    return output;
  }

};

}  // namespace

HALIDE_REGISTER_GENERATOR(ConvolutionKernel, attention)
