/*
 * An application for applying a Gaussian blur.
 * It uses a 3x3 stencil with constant weights.
 */

#include "Halide.h"

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

// Size of blur for gradients.
const int fx = 3;
const int fy = 3;
const int ic = 4;
const int ox = 16;
const int oy = 24;
const int oc = 6;
const int stride = 1;

class ConvLayer : public Halide::Generator<ConvLayer> {
public:
    Input<Buffer<int16_t>>  input{"input", 3};
    Input<Buffer<int16_t>>  weight{"weight", 4};
    Output<Buffer<int16_t>> output{"output", 3};

    void generate() {
        /* THE ALGORITHM */

        // defined variables (c: inner channel, k: outer channel)
        Var x("x"), y("y"), c("c"), k("k");
        // tiled variables
        Var xo("xo"), yo("yo"), xi("xi"), yi("yi"), co("co"), ci("ci"), ko("ko"), ki("ki");

        // Create a reduction domain of the correct bounds.
        RDom win(0, fx, 0, fy, 0, ic);

        Func hw_input, input_copy;
        input_copy(c, x, y) = cast<int16_t>(input(c, x, y));
        hw_input(c, x, y) = input_copy(c, x, y);

        Func hw_weight, weight_copy;
        weight_copy(c, k, x, y) = cast<int16_t>(weight(c, k, x, y));
        hw_weight(c, k, x, y) = weight_copy(c, k, x, y);

        Func conv;
        conv(x, y, k) = i16(0);
        conv(x, y, k) += hw_weight(win.z, k, win.x, win.y) *
          hw_input(win.z, stride*x + win.x, stride*y + win.y);

        Func hw_output;
        hw_output(x, y, k) = conv(x, y, k);
        output(x, y, k) = i16( hw_output(x, y, k) );

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR)) {

        } else if (get_target().has_feature(Target::Clockwork) || get_target().has_feature(Target::Pono)) {
          hw_input.bound(c, 0, ic);
          hw_output.bound(x, 0, ox);
          hw_output.bound(y, 0, oy);
          hw_output.bound(k, 0, oc);
          
          hw_output.compute_root();

          hw_output//.tile(x, y, xo, yo, xi, yi, 16, 24)
            .split(x, xo, xi, ox)
            .split(y, yo, yi, oy)
            .split(k, ko, ki, oc)
            .reorder(xi, yi, ki, xo, yo, ko)
            .hw_accelerate(xi, xo);

          conv.compute_at(hw_output, xo);

          hw_input.compute_at(hw_output, xo);
          input_copy.accelerator_input();
          input_copy.compute_root();

          hw_weight.compute_at(hw_output, xo);
          weight_copy.accelerator_input();
          weight_copy.compute_root();
          
        } else {    // schedule to CPU

        }
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(ConvLayer, conv_layer_3D)

