#include "Halide.h"

namespace {

enum class BlurGPUSchedule {
    Inline,          // Fully inlining schedule.
    Cache,           // Schedule caching intermedia result of blur_x.
    Slide,           // Schedule enabling sliding window opt within each
                     // work-item or cuda thread.
    SlideVectorize,  // The same as above plus vectorization per work-item.
};

std::map<std::string, BlurGPUSchedule> blurGPUScheduleEnumMap() {
    return {
        {"inline", BlurGPUSchedule::Inline},
        {"cache", BlurGPUSchedule::Cache},
        {"slide", BlurGPUSchedule::Slide},
        {"slide_vector", BlurGPUSchedule::SlideVectorize},
    };
};

class HalideBlur : public Halide::Generator<HalideBlur> {
public:
    GeneratorParam<BlurGPUSchedule> schedule{
        "schedule",
        BlurGPUSchedule::SlideVectorize,
        blurGPUScheduleEnumMap()};
    GeneratorParam<int> tile_x{"tile_x", 32};  // X tile.
    GeneratorParam<int> tile_y{"tile_y", 8};   // Y tile.

    Input<Buffer<uint16_t>> input{"input", 2};
    Output<Buffer<uint16_t>> output{"output", 2};

    GeneratorParam<uint8_t> schedule_j{"schedule_j", 2};    // default: 0

    void generate() {
        Func blur_x("blur_x");
        Func blur_y("blur_y");
        Var x("x"), y("y"), xo("xo"), yo("yo"), xi("xi"), yi("yi");

        // (original algorithm) // The algorithm
        // (original algorithm) blur_x(x, y) = (input(x, y) + input(x + 1, y) + input(x + 2, y)) / 3;
        // (original algorithm) blur_y(x, y) = (blur_x(x, y) + blur_x(x, y + 1) + blur_x(x, y + 2)) / 3;

        // The algorithm
        Func hw_input("hw_input");
        Func hw_output("hw_output");
        hw_input(x, y) = cast<uint16_t>(input(x, y));
        blur_x(x, y) = ((hw_input(x, y) + hw_input(x + 1, y) + hw_input(x + 2, y)) * 85) >> 8;
        blur_y(x, y) = ((blur_x(x, y) + blur_x(x, y + 1) + blur_x(x, y + 2)) * 85) >> 8;
        hw_output(x, y) = blur_y(x, y);
        output(x, y) = cast<uint16_t>(hw_output(x, y));

        // How to schedule it
        if (get_target().has_gpu_feature()) {
            // GPU schedule.
            switch (schedule) {
            case BlurGPUSchedule::Inline:
                // - Fully inlining.
                blur_y.gpu_tile(x, y, xi, yi, tile_x, tile_y);
                break;
            case BlurGPUSchedule::Cache:
                // - Cache blur_x calculation.
                blur_y.gpu_tile(x, y, xi, yi, tile_x, tile_y);
                blur_x.compute_at(blur_y, x).gpu_threads(x, y);
                break;
            case BlurGPUSchedule::Slide: {
                // - Instead caching blur_x calculation explicitly, the
                //   alternative is to allow each work-item in OpenCL or thread
                //   in CUDA to calculate more rows of blur_y so that temporary
                //   blur_x calculation is re-used implicitly. This achieves
                //   the similar schedule of sliding window.
                Var y_inner("y_inner");
                blur_y
                    .split(y, y, y_inner, tile_y)
                    .reorder(y_inner, x)
                    .unroll(y_inner)
                    .gpu_tile(x, y, xi, yi, tile_x, 1);
                break;
            }
            case BlurGPUSchedule::SlideVectorize: {
                // Vectorization factor.
                int factor = sizeof(int) / sizeof(short);
                Var y_inner("y_inner");
                blur_y.vectorize(x, factor)
                    .split(y, y, y_inner, tile_y)
                    .reorder(y_inner, x)
                    .unroll(y_inner)
                    .gpu_tile(x, y, xi, yi, tile_x, 1);
                break;
            }
            default:
                break;
            }
        } else if (get_target().has_feature(Target::Clockwork)) {
          // Clockwork Schedule
          if (schedule_j == 4) { // small resolution
            const int inputSize = 64;
            const int blockSize = 3;
            const int outputSize = inputSize-blockSize+1;
            const int tileSize = outputSize; // single tile
            
            output.bound(x, 0, outputSize);
            output.bound(y, 0, outputSize);

            hw_output.compute_root();

            hw_output
              .tile(x, y, xo, yo, xi, yi, tileSize, tileSize)
              .hw_accelerate(xi, xo);

            hw_input.stream_to_accelerator();

          }
          else if (schedule_j == 2) { // big resolution
            const int tileWidth = 94;
            const int tileHeight = 62;
            const int numHostTiles = 9;
            const int numTiles = 7;
            const int glbWidth = tileWidth * numTiles;
            const int glbHeight = tileHeight * numTiles;
            const int outputWidth = numHostTiles * glbWidth;
            const int outputHeight = numHostTiles * glbHeight;
            
            output.bound(x, 0, outputWidth);
            output.bound(y, 0, outputHeight);

            hw_output.in().compute_root();

            hw_output.in()
              .tile(x, y, xo, yo, xi, yi, glbWidth, glbHeight)
              .hw_accelerate(xi, xo);

            //Var xii, yii, xio, yio;
            hw_output
              .tile(x, y, xo, yo, xi, yi, tileWidth, tileHeight);
            hw_output.compute_at(hw_output.in(), xo);
            hw_output.store_in(Halide::MemoryType::GLB);
            
            // Is this necessary?
            // blur_y.compute_at(hw_output, xo);

            hw_input.in().compute_at(hw_output.in(), xo);
            hw_input.in().store_in(Halide::MemoryType::GLB);
            
            hw_input.compute_root()
              .accelerator_input();
          }

        } else {
            // CPU schedule.
            // Compute blur_x as needed at each vector of the output.
            // Halide will store blur_x in a circular buffer so its
            // results can be re-used.
            blur_y
                .compute_root()
                .split(y, y, yi, 32)
                .parallel(y)
                .vectorize(x, 16);
            blur_x
                .store_at(blur_y, y)
                .compute_at(blur_y, x)
                .vectorize(x, 16);
        }
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(HalideBlur, amber_blur)
