#include "Halide.h"

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

class UnitTestBoolean : public Halide::Generator<UnitTestBoolean> {
public:
    Input<Buffer<uint8_t>>  input{"input", 2};
    Output<Buffer<uint8_t>> output{"output", 2};

    void generate() {
        /* THE ALGORITHM */

        Var x("x"), y("y");

       Func hw_input("hw_input");
        hw_input(x, y) = u16(input(x, y));

        Func lt1, lt2;
        lt1(x,y) = hw_input(x,y) < 128;
        lt2(x,y) = hw_input(x,y) < 64;
        
        Func bool_and, bool_or, bool_not, bool_xor;
        bool_and(x,y) = lt1(x,y) && lt2(x,y);
        bool_not(x,y) = !( lt1(x,y) );
        bool_or(x,y)  = bool_and(x,y) || bool_not(x,y);
        bool_xor(x,y) = bool_or(x,y) ^ bool_not(x,y);

        Func hw_output("hw_output");
        hw_output(x, y) = select(bool_xor(x, y), i16(200), i16(0));
        output(x, y) = u8(hw_output(x,y));

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR)) {
          output.bound(x, 0, 64);
          output.bound(y, 0, 64);
          
          Var xi,yi, xo,yo;
          
          hw_output.compute_root();
          
          hw_output.tile(x,y, xo,yo, xi,yi, 64, 64)
            .hw_accelerate(xi, xo);

          hw_input.stream_to_accelerator();
          
        } else if (get_target().has_feature(Target::Clockwork) || get_target().has_feature(Target::Pono)) {
          Var xi,yi, xo,yo;

          output.bound(x, 0, 64);
          output.bound(y, 0, 64);
          
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

HALIDE_REGISTER_GENERATOR(UnitTestBoolean, boolean_ops)
