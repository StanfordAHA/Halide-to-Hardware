#include "Halide.h"

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

float gamma = 1.0f;
float contrast = 1.0f;

class UnitTestROM : public Halide::Generator<UnitTestROM> {
public:
    Input<Buffer<uint8_t>>  input{"input", 2};
    Output<Buffer<uint8_t>> output{"output", 2};

    void generate() {
        /* THE ALGORITHM */

        Var x("x"), y("y");

        Func hw_input("hw_input");
        hw_input(x, y) = cast<int16_t>(input(x, y)) * 7; // large rom (1024)
        //hw_input(x, y) = cast<int16_t>(input(x, y)); // small rom (512)

        Func curve, lookup;
        {
          Expr xf = x/1024.0f;
          Expr g = pow(xf, 1.0f/gamma);
          Expr b = 2.0f - (float) pow(2.0f, contrast/100.0f);
          Expr a = 2.0f - 2.0f*b;
          Expr val = select(g > 0.5f,
                            1.0f - (a*(1.0f-g)*(1.0f-g) + b*(1.0f-g)),
                            a*g*g + b*g);
          curve(x) = cast<uint16_t>(clamp(val*256.0f, 0.0f, 255.0f));
        }
        
        Expr in_val = clamp(hw_input(x, y), i16(0), i16(1023));
        lookup(x,y) = curve(in_val);

        Func hw_output("hw_output");
        hw_output(x, y) = lookup(x, y);
        output(x, y) = cast<uint8_t>(hw_output(x,y));

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR)) {
          Var xi,yi, xo,yo;

          output.bound(x, 0, 64);
          output.bound(y, 0, 64);

          hw_output.compute_root();
          
          hw_output.tile(x,y, xo,yo, xi,yi, 64, 64)
            .hw_accelerate(xi, xo);

          curve.compute_at(hw_output, yi).unroll(x);  // synthesize curve to a ROM

          hw_input.compute_at(hw_output, xi).store_at(hw_output, xo);
          hw_input.stream_to_accelerator();

        } else if (get_target().has_feature(Target::Clockwork)) {
          Var xi,yi, xo,yo;

          output.bound(x, 0, 64);
          output.bound(y, 0, 64);

          hw_output.compute_root();
          
          hw_output.tile(x,y, xo,yo, xi,yi, 64, 64)
            .hw_accelerate(xi, xo);

          curve.compute_at(hw_output, yi).unroll(x);  // synthesize curve to a ROM

          hw_input.stream_to_accelerator();

          
        } else {  // schedule to CPU
          output.compute_root();
        }
        
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(UnitTestROM, rom)
