#include "Halide.h"
#include "complex.h"
#include <math.h>

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

class LensDistortion : public Halide::Generator<LensDistortion> {
public:
    Input<Buffer<uint8_t>> input{"input", 2};
    Output<Buffer<uint8_t>> output{"output", 2};

    void generate() {
        // approximately calculates lens distortion

        // parameters
        float d = 15.0f; // distance (mm) from lens to screen
        float pix = .4f; // pix to mm
        int height = 64;
        int width = 64;
        float k1 = -0.25349f;
        float k2 = 0.11868f;

        Func input_copy("input_copy");
        Func hw_input("hw_input");
        Func radius2("radius2");
        Func distortion_x("distortion_x");
        Func distortion_y("distortion_y");
        Func accum("accum");
        Func count("count");
        Func hw_output("hw_output");
        Func output_copy("output_copy");
        Var x("x"), y("y");

        Var xi, xo, yi, yo;

        // The algorithm

        input_copy(x,y) = cast<int16_t>(input(x,y));
        hw_input(x,y) = input_copy(x,y);
        
        radius2(x,y) = i16(((x-width/2)*(x-width/2) + (y-height/2)*(y-height/2)) * pix*pix/(d*d));

        distortion_x(x,y) = x*(1 + k1 * radius2(x,y) + k2 * radius2(x,y) * radius2(x,y));
        distortion_y(x,y) = y*(1 + k2 * radius2(x,y) + k2 * radius2(x,y) * radius2(x,y));

        accum(x,y) = i16(0);
        accum(x,y) += Halide::select(abs(distortion_x(x,y)-(x)) < 0.5f,
                                     Halide::select(abs(distortion_y(x,y)-(y)) < 0.5f, hw_input(x,y), i16(0)), i16(0));
        count(x,y) = i16(0);
        count(x,y) += Halide::select(abs(distortion_x(x,y)-(x)) < 0.5f,
                                     Halide::select(abs(distortion_y(x,y)-(y)) < 0.5f, i16(1), i16(0)), i16(0));
        
        output_copy(x,y) = select(count(x,y)==0, i16(0), accum(x,y) / (count(x,y) + 0.01f));

        hw_output(x,y) = output_copy(x,y);
        output(x,y) = u8(hw_output(x,y));

        if (auto_schedule) {
        } else if (get_target().has_feature(Target::CoreIR)) {
        } else if (get_target().has_feature(Target::Clockwork) || get_target().has_feature(Target::Pono)) {
            output.bound(x, 0, width);
            output.bound(y, 0, height);

            hw_output.compute_root();
            hw_output.tile(x, y, xo, yo, xi, yi, width, height)
                .hw_accelerate(xi, xo);

            input_copy.accelerator_input();

            input_copy.compute_root();
        } else {
            output.compute_root();
        }
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(LensDistortion, lens_distortion)
