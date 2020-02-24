/*
 * An example Halide pipeline
 */

#include "Halide.h"

namespace {
    using namespace Halide;

    class ExamplePipeline : public Halide::Generator<ExamplePipeline> {
    public:
        Input<Buffer<uint8_t>> input {"input", 3};
        Output<Buffer<uint8_t>> output {"output", 3};

        void generate() {
            // specify algorithm
            create_algorithm();

            // specify schedule
            if(get_target().has_feature(Target::CoreIR)) {
                schedule_coreir();
            } else if(get_target().has_feature(Target::HLS)) {
                schedule_hls();
            } else {
                schedule_cpu();
            }
        }

        /*
         * Specify the algorithm
         */
        void create_algorithm() {
            bounded(c, x, y) = input(x+4, y+4, c);
            gray(x, y) = cast<uint8_t>((77 * cast<uint16_t>(bounded(0, x, y)) +
                                        150 * cast<uint16_t>(bounded(1, x, y)) +
                                        29 * cast<uint16_t>(bounded(2, x, y))) >> 8);

            sharpen(x, y) = cast<uint8_t>(clamp(2 * cast<uint16_t>(gray(x, y)), 0, 255));
            ratio(x, y) = cast<uint8_t>(clamp(cast<uint16_t>(sharpen(x, y)) * 32 / max(gray(x, y), 1), 0, 255));
            hw_output(c, x, y) = cast<uint8_t>(clamp(cast<uint16_t>(ratio(x, y)) * bounded(c, x, y) / 32, 0, 255));
            output(x, y, c) = hw_output(c, x, y);
            output.bound(c, 0, 3);
        }

        /*
         * CPU Schedule
         */
        void schedule_cpu() {
            gray.compute_at(output, xo).vectorize(x, 8);
            ratio.compute_at(output, xo).vectorize(x, 8);
            output.tile(x, y, xo, yo, xi, yi, 240, 360);
            output.vectorize(xi, 8).reorder(xi, yi, c, xo, yo);
            output.fuse(xo, yo, xo).parallel(xo);
        }

        /*
         * HLS Schedule
         */
        void schedule_hls() {
            bounded.compute_at(output, xo);
            bounded.fifo_depth(hw_output, 64*9);
            gray.linebuffer().fifo_depth(hw_output, 64*9);
            ratio.linebuffer();
            hw_output.compute_at(output, xo);
            hw_output.tile(x, y, xo, yo, xi, yi, 64, 64).reorder(c, xi, yi, xo, yo);
            hw_output.unroll(c);
            hw_output.accelerate({bounded}, xi, xo);
            output.tile(x, y, xo, yo, xi, yi, 64, 64).reorder(c, xi, yi, xo, yo);
        }

        /*
         * CoreIR Schedule
         */
        void schedule_coreir() {
            schedule_hls();
        }

    private:
        Func bounded;
        Func gray;
        Func ratio;
        Func sharpen;
        Func hw_output;

        Var x, y, c;
        Var xo, yo, xi, yi;
    };
}

HALIDE_REGISTER_GENERATOR(ExamplePipeline, example)

