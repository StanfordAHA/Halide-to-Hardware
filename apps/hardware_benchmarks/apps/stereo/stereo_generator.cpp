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

    //int windowR = 8;
    int windowR = 4;
    int searchR = 12;

   

    void generate() {
      
      
      Func left_padded, right_padded;

      Func SAD, offset0, offset1, hw_output;
      RDom win(-windowR, windowR*2, -windowR, windowR*2);
      RDom search(0, searchR);

      right_padded = Halide::BoundaryConditions::repeat_edge(left);
      left_padded = Halide::BoundaryConditions::repeat_edge(right);


      Func hw_right_input("hw_right_input"), hw_right_input_copy("hw_right_input_copy");
      hw_right_input(x, y) = cast<uint16_t>(right_padded(x, y));
      hw_right_input_copy(x,y) = hw_right_input(x, y);

      Func hw_left_input("hw_left_input"), hw_left_input_copy("hw_left_input_copy");
      hw_left_input(x, y) = cast<uint16_t>(left_padded(x, y));
      hw_left_input_copy(x,y) = hw_left_input(x, y);

      SAD(x, y, c) = cast<uint16_t>(0);
      SAD(x, y, c) += cast<uint16_t>(absd(hw_right_input_copy(x+win.x, y+win.y),
                                          hw_left_input_copy(x+win.x+5+c, y+win.y)));

      offset0(x, y) = cast<int16_t>(0);
      offset1(x, y) = cast<uint16_t>(65535);

      offset1(x,y) = min(SAD(x, y, search.x), offset1(x, y));
      offset0(x, y) = select(SAD(x, y, search.x) == offset1(x, y), cast<int16_t>(search.x), offset0(x, y));

      Func offset_out("offset_out");
      offset_out(x, y) = offset0(x, y);
      // offset_out(x, y) = SAD(x, y, 0)/256;

      hw_output(x, y) = cast<uint16_t>(offset_out(x, y)) * 255 / 8;
      // hw_output(x, y) = cast<uint16_t>(SAD(x, y, 0)/256);
      output(x, y) = cast<uint8_t>(hw_output(x, y));

      output.bound(x, 0, 128);
      output.bound(y, 0, 128);


      if (get_target().has_feature(Target::Clockwork)) {
        hw_output.compute_root();
        hw_output.tile(x, y, xo, yo, xi, yi, 128, 128).hw_accelerate(xi, xo);
        
        offset_out.compute_at(hw_output, xo).store_at(hw_output, xo);
        offset0.compute_at(hw_output, xo).store_at(hw_output, xo);
        offset0.update().unroll(search.x);
        offset1.compute_at(hw_output, xo).store_at(hw_output, xo);
        offset1.update().unroll(search.x);


        //SAD.reorder(c, x, y);
        SAD.update().reorder(x, y, win.x, win.y);
        SAD.compute_at(hw_output, xo).store_at(hw_output, xo);
        //SAD.update()
        //  .unroll(win.x).unroll(win.y);


        // right_padded.compute_at(hw_output, xo).store_at(hw_output, xo);
        // left_padded.compute_at(hw_output, xo).store_at(hw_output, xo);


        hw_right_input_copy.compute_at(hw_output, xo);
        hw_left_input_copy.compute_at(hw_output, xo);
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
