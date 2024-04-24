#include "Halide.h"

namespace {

using namespace std;

using namespace Halide;
using namespace Halide::ConciseCasts;

class LtCompare : public Halide::Generator<LtCompare> {
public:
    Input<Buffer<uint16_t>>  input{"input", 3};
    Output<Buffer<uint16_t>> output{"output", 3};

    GeneratorParam<int> out_img{"out_img", 10};
    GeneratorParam<int> n_oc{"n_oc", 8};
  
    GeneratorParam<int32_t> myunroll{"myunroll", 1};
  
    void generate() {
        /* THE ALGORITHM */

      Var x("x"), y("y"), w("w");
      Func hw_input("hw_input"), clamp6, hw_output, lt;

      hw_input(w, x, y) = cast<bfloat16_t>(input(w, x, y));
        
      // relu(w, x, y) = max(hw_input(w,x,y), cast<bfloat16_t>(0.0f));
      lt(w, x, y) = hw_input(w,x,y) < Expr(bfloat16_t(6.0f));
      // relu(w, x, y) = min(hw_input(w,x,y), cast<bfloat16_t>(6.0f));
      clamp6(w, x, y) = select(lt(w,x,y), hw_input(w,x,y), bfloat16_t(6.0f));
      
      // hw_output(w, x, y) = min(relu(w, x, y), cast<bfloat16_t>(6.0f));
      hw_output(w, x, y) = clamp6(w, x, y);
      output(w, x, y) = cast<uint16_t>(hw_output(w, x, y));

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR) ||
            get_target().has_feature(Target::HLS)) {

        } else if (get_target().has_feature(Target::Clockwork)) {
          Var xi,yi, xo,yo;

          output.bound(x, 0, out_img);
          output.bound(y, 0, out_img);
          output.bound(w, 0, n_oc);
          
          //hw_input.compute_root();
          hw_output.compute_root();
          hw_output
            .tile(x,y, xo,yo, xi,yi, out_img, out_img)
            .reorder(w, xi, yi, xo, yo)
            .hw_accelerate(xi, xo);
          hw_output.unroll(w, myunroll);

          clamp6.compute_at(hw_output, xo)
            .unroll(w, myunroll);
          
          hw_input.stream_to_accelerator();
          hw_input.in().unroll(w, myunroll);
          
        } else {

        }
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(LtCompare, lt_compare_fp)
