#include "Halide.h"

namespace {

  using namespace Halide;

      Var x("x"), y("y");
      Var xo("xo"), yo("yo"), xi("xi"), yi("yi"), z("z"), c("c");

  class StereoPipeline : public Halide::Generator<StereoPipeline> {  
  public:
    Input<Buffer<uint8_t>> left{"left", 2};
    Input<Buffer<uint8_t>> right{"right", 2};
    Output<Buffer<uint8_t>> output{"output", 2};

    int windowR = 4;
    int searchR = 10;

    Func rectify_float(Func img, Func remap) {
      Expr offsetX = cast<float>(cast<int16_t>(remap(x, y, 0)) - cast<int16_t>(128)) / 16.0f;
      Expr offsetY = cast<float>(cast<int16_t>(remap(x, y, 1)) - cast<int16_t>(128)) / 16.0f;

      Expr targetX = cast<int>(floor(offsetX));
      Expr targetY = cast<int>(floor(offsetY));

      Expr wx = offsetX - targetX;
      Expr wy = offsetY - targetY;

      Func interpolated("interpolated");
      interpolated(x, y) = lerp(lerp(img(x+targetX, y+targetY, 1), img(x+targetX+1, y+targetY, 1), wx),
                                lerp(img(x+targetX, y+targetY+1, 1), img(x+targetX+1, y+targetY+1, 1), wx), wy);

      return interpolated;
    }

    Func rectify_int(Func img, Func remap) {
      Expr targetX = (cast<int16_t>(remap(x, y, 0)) - 128) / 16;
      Expr targetY = (cast<int16_t>(remap(x, y, 1)) - 128) / 16;

      // wx and wy are equal to wx*256 and wy*256 in rectify_float()
      Expr wx = cast<uint8_t>((cast<int16_t>(remap(x, y, 0)) * 16) - (128 * 16) - (targetX * 256));
      Expr wy = cast<uint8_t>((cast<int16_t>(remap(x, y, 1)) * 16) - (128 * 16) - (targetY * 256));

      Func interpolated("interpolated");
      interpolated(x, y) = lerp(lerp(img(x+targetX, y+targetY, 1), img(x+targetX+1, y+targetY, 1), wx),
                                lerp(img(x+targetX, y+targetY+1, 1), img(x+targetX+1, y+targetY+1, 1), wx), wy);

      return interpolated;
    }

    void generate() {
      
      
      Func left_padded, right_padded;
      Func left_remapped, right_remapped;
      Func SAD, offset0, offset1, hw_output;
      RDom win(-windowR, windowR*2, -windowR, windowR*2);
      RDom search(0, searchR);

      Func hw_right_input;
      hw_right_input(x, y) = cast<uint16_t>(right(x, y));

      Func hw_left_input;
      hw_left_input(x, y) = cast<uint16_t>(left(x, y));

      right_padded = BoundaryConditions::constant_exterior(hw_right_input, 0);
      left_padded = BoundaryConditions::constant_exterior(hw_left_input, 0);

      right_remapped = right_padded;
      left_remapped = left_padded;


      SAD(x, y, c) = 0;
      SAD(x, y, c) += cast<uint16_t>(absd(right_remapped(x+win.x, y+win.y),
                                          left_remapped(x+win.x+20+c, y+win.y)));


      offset0(x, y) = cast<uint16_t>(0);
      offset1(x, y) = cast<uint16_t>(65535);
      offset1(x, y) = cast<uint16_t>(min(SAD(x, y, search.x), offset1(x, y)));
      offset0(x, y) = cast<uint16_t>(select(SAD(x, y, search.x) < offset1(x, y), cast<uint16_t>(search.x), offset0(x, y)));

      hw_output(x, y) = cast<uint8_t>(cast<uint16_t>(offset0(x, y)) * 255 / searchR);

      output(x, y) = hw_output(x, y);

      output.bound(x, 0, 128);
      output.bound(y, 0, 128);


      if (get_target().has_feature(Target::Clockwork)) {
        hw_output.compute_root();
        hw_output.tile(x, y, xo, yo, xi, yi, 128, 128).hw_accelerate(xi, xo);
        
        offset0.compute_root();
        offset1.compute_root();

        SAD.compute_root();

        right_remapped.compute_root();
        left_remapped.compute_root();

        right_padded.compute_root();
        left_padded.compute_root();

        hw_right_input.stream_to_accelerator();
        hw_left_input.stream_to_accelerator();
        // RVar so("so"), si("si");
        //offset.update(0).unroll(search.x, 16); // the unrolling doesn's generate code that balances the computation, creating a long critical path
        //offset.update(0).split(search.x, so, si, 16);
        //SAD.compute_at(offset, so);
        // SAD.unroll(c);
        // SAD.update(0).unroll(win.x).unroll(win.y).unroll(c);

      }
    }
  };

} // namespace

HALIDE_REGISTER_GENERATOR(StereoPipeline, stereo)
