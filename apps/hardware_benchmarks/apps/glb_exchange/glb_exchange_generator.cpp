#include "Halide.h"

namespace {

  using namespace std;
  using namespace Halide;
  using namespace Halide::ConciseCasts;

  class GLB_Exchange : public Halide::Generator<GLB_Exchange> {
  public:
    Input<Buffer<uint16_t>>  input{"input", 3};
    Output<Buffer<uint16_t>> output{"output", 3};

    GeneratorParam<int> out_img{"out_img", 56};
    GeneratorParam<int> n_oc{"n_oc", 32};

    // pad_o determines the output padding
    GeneratorParam<int> pad_o_left{"pad_o_left", 0};    // default: 0
    GeneratorParam<int> pad_o_right{"pad_o_right", 0};    // default: 0

    GeneratorParam<int> input_num_tiles{"input_num_tiles", 4};
    GeneratorParam<int> output_num_tiles{"output_num_tiles", 8};

    void generate() {

      /* THE ALGORITHM */
      Var x("x"), y("y"), w("w");
      Func hw_input("hw_input"), input_host("input_host"), input_glb("input_glb"), input_cgra("input_cgra");
      Func hw_output("hw_output"), output_glb("output_glb"), output_cgra("output_cgra");

      hw_input(w, x, y) = input(w, x, y);
      output_cgra(w, x, y) = hw_input(w, x, y);
      hw_output(w, x, y) = output_cgra(w, x, y);
      output(w, x, y) = hw_output(w, x, y);

      /* THE SCHEDULE */
      if (get_target().has_feature(Target::Clockwork)) {

        Var xi,yi, xo,yo;
        output.bound(x, 0, out_img);
        output.bound(y, 0, out_img);
        output.bound(w, 0, n_oc);
        
        //hw_input.compute_root();
        hw_output.compute_root();
        hw_output
          .tile(x, y, xo, yo, xi, yi, out_img, out_img)
          .reorder(w, xi, yi, xo, yo)
          .hw_accelerate(xi, xo);
        hw_output.unroll(w, output_num_tiles);

        output_cgra.compute_at(hw_output, xo).unroll(w, output_num_tiles);
        
        hw_input.stream_to_accelerator();
        hw_input.in().unroll(w, input_num_tiles);
        
      } else {

      }
    }
  };
}

HALIDE_REGISTER_GENERATOR(GLB_Exchange, glb_exchange)
