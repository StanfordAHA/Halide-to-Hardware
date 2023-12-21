#include "Halide.h"

namespace {

using namespace Halide::ConciseCasts;
using namespace Halide::BoundaryConditions;

class Max : public Halide::Generator<Max> {
public:
  GeneratorParam<int> radius_{"radius", 3}; //26
  Input<Buffer<uint8_t>> input_{"input", 3};
  Output<Buffer<uint8_t>> output{"output", 3};

  int inputSize = 60;
  int input_height = inputSize;

  void generate() {
    Var x("x"), y("y"), c("c"), t("t");

    Func input = repeat_edge(input_,
                             {{input_.dim(0).min(), input_.dim(0).extent()},
                              {input_.dim(1).min(), input_.dim(1).extent()}});

    Func hw_input;
    hw_input(x, y, c) = cast<uint16_t>(input(x, y, c));
        
    const int radius = radius_;
    const int slices = (int)(ceilf(logf(radius) / logf(2))) + 1;

    // A sequence of vertically-max-filtered versions of the input,
    // each filtered twice as tall as the previous slice. All filters
    // are downward-looking.
    Func vert_log("vert_log");
    vert_log(x, y, c, t) = hw_input(x, y, c);
    RDom r(-radius, input_height + radius, 1, slices - 1);
    vert_log(x, r.x, c, r.y) = max(vert_log(x, r.x, c, r.y - 1),
                                   vert_log(x, r.x + clamp((1 << (r.y - 1)), 0, radius * 2), c, r.y - 1));

    // We're going to take a max filter of arbitrary diameter
    // by maxing two samples from its floor log 2 (e.g. maxing two
    // 8-high overlapping samples). This next Func tells us which
    // slice to draw from for a given radius:
    Func slice_for_radius("slice_for_radius");
    slice_for_radius(t) = cast<int>(floor(log(2 * t + 1) / logf(2)));

    // Produce every possible vertically-max-filtered version of the image:
    Func vert("vert"), slice("slice");
    // t is the blur radius
    slice(t) = clamp(slice_for_radius(t), 0, slices);
    Expr first_sample = vert_log(x, y - t, c, slice(t));
    Expr second_sample = vert_log(x, y + t + 1 - clamp(1 << slice(t), 0, 2 * radius), c, slice(t));
    vert(x, y, c, t) = max(first_sample, second_sample);

    Func filter_height("filter_height");
    RDom dy(0, radius + 1);
    filter_height(x) = sum(select(x * x + dy * dy < (radius * radius), 1, 0));

    // Now take an appropriate horizontal max of them at each output pixel
    Func hw_output;
    RDom dx(-radius, 2 * radius + 1);
    hw_output(x, y, c) = maximum(vert(x + dx, y, c, clamp(filter_height(dx), 0, radius + 1)));
    output(x, y, c) = cast<uint8_t>(hw_output(x, y, c));

    // Schedule
    if (!auto_schedule) {
      if (get_target().has_feature(Target::Clockwork) || get_target().has_feature(Target::Pono)) {
        int outputSize = 64;
        int tileSize = 64;
            
        output.bound(x, 0, outputSize);
        output.bound(y, 0, outputSize);
        output.bound(c, 0, 3);

        hw_output.compute_root();

        Var xi, xo, yi, yo;
        hw_output
          .tile(x, y, xo, yo, xi, yi, tileSize, tileSize)
          .reorder(c, xi, yi, xo, yo)
          .hw_accelerate(xi, xo);
        
        hw_output.unroll(c);

        filter_height.compute_at(hw_output, xo);
        vert.compute_at(hw_output, xo);
        vert_log.compute_at(hw_output, xo);

        //slice_for_radius.compute_at(hw_output, xo).unroll(t);
        slice.compute_at(hw_output, xo).unroll(t);
        
        hw_input.stream_to_accelerator();
      } else if (get_target().has_gpu_feature()) {
        // 11.8ms on a 2060 RTX

        slice_for_radius.compute_root();
        filter_height.compute_root();
        Var xi, xo, yi;

        output.gpu_tile(x, y, xi, yi, 32, 8)
          .gpu_blocks(x, y, c);

        // There's a boundary condition on the input, so let's
        // keep things simple with a RoundUp tail strategy.
        vert_log.compute_root()
          .reorder(c, t, x, y)
          .gpu_tile(x, y, xi, yi, 16, 16, TailStrategy::RoundUp)
          .update()
          .split(x, xo, xi, 32, TailStrategy::RoundUp)
          .reorder(r.x, r.y, xi, xo, c)
          .gpu_blocks(xo, c)
          .gpu_threads(xi);

      } else {
        // 47ms on an Intel i9-9960X using 16 threads

        Var tx;
        // These don't matter, just LUTs
        slice_for_radius.compute_root();
        filter_height.compute_root();

        // vert_log.update() doesn't have enough parallelism, but I
        // can't figure out how to give it more... Split whole image
        // into slices.

        output.compute_root()
          .split(x, tx, x, 256)
          .reorder(x, y, c, tx)
          .fuse(c, tx, t)
          .parallel(t)
          .vectorize(x, 8);
        vert_log.compute_at(output, t);
        vert_log.vectorize(x, 8);
        vert_log.update()
          .reorder(x, r.x, r.y, c)
          .vectorize(x, 8);
        vert.compute_at(output, y)
          .vectorize(x, 8);
      }
    }
  }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(Max, max_filter)
