#include "Halide.h"

namespace {

using namespace Halide;

class UnitTestBoolean : public Halide::Generator<UnitTestBoolean> {
public:
    Input<Buffer<int16_t>>  input{"input", 2};
    Output<Buffer<int16_t>> output{"output", 2};

    void generate() {
        /* THE ALGORITHM */

        Var x("x"), y("y");

        Func hw_input("hw_input");
        hw_input(x, y) = cast<int16_t>(input(x, y));

        Func lt1, lt2;
        lt1(x,y) = hw_input(x,y) < 128;
        lt2(x,y) = hw_input(x,y) < 64;
        
	Func bool_and, bool_or, bool_not, bool_xor;
        bool_and(x,y) = lt1(x,y) && lt2(x,y);
        bool_not(x,y) = !( lt1(x,y) );
        bool_or(x,y)  = bool_and(x,y) || bool_not(x,y);
        bool_xor(x,y) = bool_or(x,y) ^ bool_not(x,y);

        Func hw_output("hw_output");
        hw_output(x, y) = cast<int16_t>(select(bool_xor(x, y), 200, 0));
        output(x, y) = hw_output(x,y);

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR)) {
          Var xi,yi, xo,yo;
          
          hw_input.compute_root();
          hw_output.compute_root();
          
          hw_output.tile(x,y, xo,yo, xi,yi, 64, 64)
            .hw_accelerate(xi, xo);

          hw_input.stream_to_accelerator();
          
        } else {  // schedule to CPU
          output.compute_root();
        }
        
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(UnitTestBoolean, boolean)
