/*
 *
 */


#include "Halide.h"

namespace {

  using namespace Halide;
  Var x("x"), y("y"), z("z"), c("c");
  Var x_grid("x_grid"), y_grid("y_grid"), xo("xo"), yo("yo"), x_in("x_in"), y_in("y_in");

  class StereoPipeline : public Halide::Generator<StereoPipeline> {  
  public:
    Input<Buffer<uint8_t>> left{"input_left", 3};
    Input<Buffer<uint8_t>> right{"input_right", 3};
    Input<Buffer<uint8_t>> left_remap{"left_remap", 3};
    Input<Buffer<uint8_t>> right_remap{"right_remap", 3};
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

    Func rectify_noop(Func img, Func remap) {
      Func pass("pass");
      pass(x, y) = img(x, y, 1);
      return pass;
    }

    void generate() {
      Func left_padded, right_padded, left_remap_padded, right_remap_padded;
      Func left_remapped, right_remapped;
      Func SAD, offset, hw_output;
      RDom win(-windowR, windowR*2, -windowR, windowR*2);
      RDom search(0, searchR);
        
      right_padded = BoundaryConditions::constant_exterior(right, 0);
      left_padded = BoundaryConditions::constant_exterior(left, 0);
      right_remap_padded = BoundaryConditions::constant_exterior(right_remap, 128);
      left_remap_padded = BoundaryConditions::constant_exterior(left_remap, 128);

      right_remapped = rectify_noop(right_padded, right_remap_padded);
      left_remapped = rectify_noop(left_padded, left_remap_padded);

      SAD(x, y, c) += cast<uint16_t>(absd(right_remapped(x+win.x, y+win.y),
                                          left_remapped(x+win.x+20+c, y+win.y)));

      //offset(x, y) = argmin(SAD(x, y, search.x));
      // avoid using the form of the inlined reduction function of "argmin",
      // so that we can get a handle for scheduling
      offset(x, y) = {cast<int8_t>(0), cast<uint16_t>(65535)};
      offset(x, y) = {select(SAD(x, y, search.x) < offset(x, y)[1],
                             cast<int8_t>(search.x),
                             offset(x, y)[0]),
                      min(SAD(x, y, search.x), offset(x, y)[1])};

      hw_output(x, y) = cast<uint8_t>(cast<uint16_t>(offset(x, y)[0]) * 255 / searchR);
      output(x, y) = hw_output(x, y);

      // all inputs has three channels
      //right_padded.bound(c, 0, 3);
      //left_padded.bound(c, 0, 3);
      //right_remap_padded.bound(c, 0, 3);
      //left_remap_padded.bound(c, 0, 3);
      SAD.bound(c, 0, 3);


      if (get_target().has_feature(Target::CoreIR)) {
        right_padded.compute_root();
        left_padded.compute_root();
        right_remap_padded.compute_root();
        left_remap_padded.compute_root();

        hw_output.compute_root();
        hw_output.tile(x, y, xo, yo, x_in, y_in, 256, 256);
        hw_output.accelerate({right_remapped, left_remapped}, x_in, xo);
        right_remapped.compute_root();
        left_remapped.compute_root();

        RVar so("so"), si("si");
        //offset.update(0).unroll(search.x, 16); // the unrolling doesn's generate code that balances the computation, creating a long critical path
        //offset.update(0).split(search.x, so, si, 16);
        //SAD.compute_at(offset, so);
        SAD.unroll(c);
        SAD.update(0).unroll(win.x).unroll(win.y).unroll(c);

      } else if (get_target().has_feature(Target::Clockwork)) {
        right_padded.compute_root();
        left_padded.compute_root();
        right_remap_padded.compute_root();
        left_remap_padded.compute_root();

        hw_output.compute_root();
        hw_output.tile(x, y, xo, yo, x_in, y_in, 256, 256);
        hw_output.accelerate({right_remapped, left_remapped}, x_in, xo);
        right_remapped.compute_root();
        left_remapped.compute_root();

        RVar so("so"), si("si");
        //offset.update(0).unroll(search.x, 16); // the unrolling doesn's generate code that balances the computation, creating a long critical path
        //offset.update(0).split(search.x, so, si, 16);
        //SAD.compute_at(offset, so);
        SAD.unroll(c);
        SAD.update(0).unroll(win.x).unroll(win.y).unroll(c);

      } else if (get_target().has_feature(Target::CoreIR)) {
        output.tile(x, y, xo, yo, x_in, y_in, 256, 64);
        output.fuse(xo, yo, xo).parallel(xo);
        //output.split(y, yo, y_in, 64).parallel(yo);

        right_padded.compute_at(output, xo).vectorize(_0, 16);
        left_padded.compute_at(output, xo).vectorize(_0, 16);
        right_remap_padded.compute_at(output, xo).vectorize(_0, 16);
        left_remap_padded.compute_at(output, xo).vectorize(_0, 16);

        //right_remapped.compute_at(output, yo);
        //left_remapped.compute_at(output, yo);

        SAD.compute_at(output, x_in);
        SAD.unroll(c);
        SAD.update(0).vectorize(c, 8).unroll(win.x).unroll(win.y);
      }
    }
  };

} // namespace

HALIDE_REGISTER_GENERATOR(StereoPipeline, stereo)
