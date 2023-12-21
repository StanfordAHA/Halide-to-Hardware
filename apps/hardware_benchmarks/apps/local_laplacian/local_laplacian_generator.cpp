#include "Halide.h"
#include "halide_trace_config.h"

namespace {

constexpr int maxJ = 20;

class LocalLaplacian : public Halide::Generator<LocalLaplacian> {
public:
    GeneratorParam<int>     pyramid_levels{"pyramid_levels", 3, 1, maxJ};

    Input<Buffer<uint16_t>> input{"input", 3};
    //Input<int>              levels{"levels"};   // sample value: 8
    //Input<float>            alpha{"alpha"};     // sample value: 1/7
    //Input<float>            beta{"beta"};       // sample value: 1

    Output<Buffer<uint16_t>> output{"output", 3};

    void generate() {
        /* THE ALGORITHM */
        const int J = pyramid_levels;

        // Make the remapping function as a lookup table.
        Func remap;
        Expr fx = cast<float>(x) / 256.0f;
        remap(x) = 1/7.f *fx*exp(-fx*fx/2.0f); //remap(x) = alpha*fx*exp(-fx*fx/2.0f);

        // Set a boundary condition
        Func clamped = Halide::BoundaryConditions::repeat_edge(input);

        // Convert to floating point
        Func floating;
        floating(x, y, c) = clamped(x, y, c) / 65535.0f;

        // Get the luminance channel
        Func gray;
        gray(x, y) = 0.299f * floating(x, y, 0) + 0.587f * floating(x, y, 1) + 0.114f * floating(x, y, 2);

        // Make the processed Gaussian pyramid.
        Func gPyramid[maxJ];
        // Do a lookup into a lut with 256 entires per intensity level
        Expr level = k * (1.0f / (8 - 1)); //Expr level = k * (1.0f / (levels - 1));
        Expr idx = gray(x, y)*cast<float>(8-1)*256.0f; //Expr idx = gray(x, y)*cast<float>(levels-1)*256.0f;
        idx = clamp(cast<int>(idx), 0, (8-1)*256); //idx = clamp(cast<int>(idx), 0, (levels-1)*256);
        gPyramid[0](x, y, k) = 1*(gray(x, y) - level) + level + remap(idx - 256*k); //gPyramid[0](x, y, k) = beta*(gray(x, y) - level) + level + remap(idx - 256*k);
        for (int j = 1; j < J; j++) {
            gPyramid[j](x, y, k) = downsample(gPyramid[j-1])(x, y, k);
        }

        // Get its laplacian pyramid
        Func lPyramid[maxJ];
        lPyramid[J-1](x, y, k) = gPyramid[J-1](x, y, k);
        for (int j = J-2; j >= 0; j--) {
            lPyramid[j](x, y, k) = gPyramid[j](x, y, k) - upsample(gPyramid[j+1])(x, y, k);
        }

        // Make the Gaussian pyramid of the input
        Func inGPyramid[maxJ];
        inGPyramid[0](x, y) = gray(x, y);
        for (int j = 1; j < J; j++) {
            inGPyramid[j](x, y) = downsample(inGPyramid[j-1])(x, y);
        }

        // Make the laplacian pyramid of the output
        Func outLPyramid[maxJ];
        Func li[maxJ];
        Func lipo[maxJ];
        for (int j = 0; j < J; j++) {
            // Split input pyramid value into integer and floating parts
            Expr level = inGPyramid[j](x, y) * cast<float>(8-1); //Expr level = inGPyramid[j](x, y) * cast<float>(levels-1);
            li[j](x, y) = clamp(cast<int>(level), 0, 8-2); //Expr li = clamp(cast<int>(level), 0, levels-2);
            Expr lf = level - cast<float>(li[j](x,y));
            lipo[j](x, y) = li[j](x, y) + 1;
            // Linearly interpolate between the nearest processed pyramid levels
            outLPyramid[j](x, y) = (1.0f - lf) * lPyramid[j](x, y, li[j](x, y)) + lf * lPyramid[j](x, y, lipo[j](x,y));
        }

        // Make the Gaussian pyramid of the output
        Func outGPyramid[maxJ];
        outGPyramid[J-1](x, y) = outLPyramid[J-1](x, y);
        for (int j = J-2; j >= 0; j--) {
            outGPyramid[j](x, y) = upsample(outGPyramid[j+1])(x, y) + outLPyramid[j](x, y);
        }

        // Reintroduce color (Connelly: use eps to avoid scaling up noise w/ apollo3.png input)
        Func color;
        float eps = 0.01f;
        color(x, y, c) = outGPyramid[0](x, y) * (floating(x, y, c)+eps) / (gray(x, y)+eps);

        // Convert back to 16-bit
        Func hw_output("hw_output");
        hw_output(x, y, c) = cast<uint16_t>(clamp(color(x, y, c), 0.0f, 1.0f) * 65535.0f);
        output(x, y, c) = hw_output(x, y, c);

        /* THE SCHEDULE */

        // Provide estimates on the input image.
        // (This can be useful in conjunction with RunGen as well
        // as auto-schedule, so we do it in all cases.)
        input.dim(0).set_bounds_estimate(0, 1536);
        input.dim(1).set_bounds_estimate(0, 2560);
        input.dim(2).set_bounds_estimate(0, 3);
        // Provide estimates on the parameters
        //levels.set_estimate(8);
        //alpha.set_estimate(1);
        //beta.set_estimate(1);
        // Provide estimates on the pipeline output
        output.estimate(x, 0, 1536)
              .estimate(y, 0, 2560)
              .estimate(c, 0, 3);

        if (auto_schedule) {
            // Nothing.
        } else if (get_target().has_gpu_feature()) {
            // gpu schedule
            remap.compute_root();
            Var xi, yi;
            output.compute_root().gpu_tile(x, y, xi, yi, 16, 8);
            for (int j = 0; j < J; j++) {
                int blockw = 16, blockh = 8;
                if (j > 3) {
                    blockw = 2;
                    blockh = 2;
                }
                if (j > 0) {
                    inGPyramid[j].compute_root().gpu_tile(x, y, xi, yi, blockw, blockh);
                    gPyramid[j].compute_root().reorder(k, x, y).gpu_tile(x, y, xi, yi, blockw, blockh);
                }
                outGPyramid[j].compute_root().gpu_tile(x, y, xi, yi, blockw, blockh);
            }
        } else if (get_target().has_feature(Target::Clockwork) || get_target().has_feature(Target::Pono)) {
            // clockwork schedule
            int tilesize = 64;
          
            output.bound(x, 0, tilesize);
            output.bound(y, 0, tilesize);
            output.bound(c, 0, 3);
            output.compute_root();          

            Var xi, yi, xo, yo;
            hw_output
              .compute_root()
              .tile(x, y, xo, yo, xi, yi, tilesize, tilesize)
              .reorder(c, xi, yi, xo, yo)
              .hw_accelerate(xi, xo);
            
            for (int j = 0; j < J; j++) {
                if (j > 0) {
                  inGPyramid[j].compute_at(hw_output, xo);
                  gPyramid[j].compute_at(hw_output, xo).reorder(k, x, y);
                  lPyramid[j].compute_at(hw_output, xo).reorder(k, x, y);
                }
                li[j].compute_at(hw_output, xo);
                lipo[j].compute_at(hw_output, xo);
                outLPyramid[j].compute_at(hw_output, xo);
                outGPyramid[j].compute_at(hw_output, xo);
            }

            clamped.stream_to_accelerator();
        } else {
            // cpu schedule
            remap.compute_root();
            Var yo;
            output.reorder(c, x, y).split(y, yo, y, 64).parallel(yo).vectorize(x, 8);
            gray.compute_root().parallel(y, 32).vectorize(x, 8);
            for (int j = 1; j < 5; j++) {
                inGPyramid[j]
                    .compute_root().parallel(y, 32).vectorize(x, 8);
                gPyramid[j]
                    .compute_root().reorder_storage(x, k, y)
                    .reorder(k, y).parallel(y, 8).vectorize(x, 8);
                outGPyramid[j]
                    .store_at(output, yo).compute_at(output, y).fold_storage(y, 8)
                    .vectorize(x, 8);
            }
            outGPyramid[0].compute_at(output, y).vectorize(x, 8);
            for (int j = 5; j < J; j++) {
                inGPyramid[j].compute_root();
                gPyramid[j].compute_root().parallel(k);
                outGPyramid[j].compute_root();
            }
        }

    }
private:
    Var x, y, c, k;

    // Downsample with a 1 3 3 1 filter
    Func downsample(Func f) {
        using Halide::_;
        Func downx, downy;
        downx(x, y, _) = (f(2*x-1, y, _) + 3.0f * (f(2*x, y, _) + f(2*x+1, y, _)) + f(2*x+2, y, _)) / 8.0f;
        downy(x, y, _) = (downx(x, 2*y-1, _) + 3.0f * (downx(x, 2*y, _) + downx(x, 2*y+1, _)) + downx(x, 2*y+2, _)) / 8.0f;
        return downy;
    }

    // Upsample using bilinear interpolation
    Func upsample(Func f) {
        using Halide::_;
        Func upx, upy;
        upx(x, y, _) = 0.25f * f((x/2) - 1 + 2*(x % 2), y, _) + 0.75f * f(x/2, y, _);
        upy(x, y, _) = 0.25f * upx(x, (y/2) - 1 + 2*(y % 2), _) + 0.75f * upx(x, y/2, _);
        return upy;
    }


};

}  // namespace

HALIDE_REGISTER_GENERATOR(LocalLaplacian, local_laplacian)
