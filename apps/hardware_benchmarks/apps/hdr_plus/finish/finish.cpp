#include "Halide.h"

namespace {

using namespace Halide::ConciseCasts;

class Finish : public Halide::Generator<Finish> {
public:
    Input<Buffer<uint8_t>>  input{"input", 3};
    Output<Buffer<uint8_t>> output{"output", 3};

    void generate() {
        // Algorithm
        
        Func clamped = Halide::BoundaryConditions::repeat_edge(input);
        Func chroma_output = chroma_denoise(clamped);
        Func dr_comp_output = dr_comp(chroma_output);
        Func gamma_correct_output = gamma_correct(dr_comp_output);
        output = gamma_correct_output;
    }

private:
    Var x, y, c;

    Func dr_comp(Func input) {
        return output;
    }

    Func gamma_correct(Func input) {
        return output;
    }

    Func chroma_denoise(Func input) {
        Func yuv = rgb_to_yuv(input); 
        Func output = bilateral_filter(yuv);
        Func rgb = yuv_to_rgb(yuv);

        return rgb;
    }

    Func bilateral_filter(Func input) {

        // Algorithm
        Func kernel_f("gauss_kernel_func");
        Func kernel_1d("gauss_kernel_1d");
        Func kernel("gauss_kernel");
        Func weight("bilateral_weight");
        Func sum_weight("bilateral_sum_weight");
        Func bilateral("bilateral");
        Func output("bilateral_filter_output");

        Var x, y, dx, dy, c, xo, yo, xi, yi, outer;
        RDom r(-3, 7, -3, 7);

        // gaussian kernel
        float sigma_gauss = 1.5f;
        kernel_f(x) = exp(-x*x/(2*sigma_gauss*sigma_gauss)) / (sqrtf(2*M_PI)*sigma_gauss);
        kernel_1d(x) = cast<uint8_t>(kernel_f(x) * 256 /
                            (kernel_f(0) + kernel_f(1)*2 + kernel_f(2)*2 + kernel_f(3)*2));
        kernel(x, y) = u16(kernel_1d(x)) * u16(kernel_1d(y));

        // weight based on intensity
        float sigma_int = 10.f;
        Expr dist = f32(i16(input(x, y, 0)) - i16(input(x + dx, y + dy, 0)));
        Expr weight_int = 65556 * exp(-dist * dist /(2*sigma_int*sigma_int));

        // TODO change floating to fixed
        weight(dx, dy, x, y) = f32(kernel(dx, dy)) * weight_int;
        sum_weight(x, y) = sum(weight(r.x, r.y, x, y));

        bilateral(x, y, c) = select( c == 0, input(x, y, c),
                i16(sum(f32(input(x+r.x, y+r.y, c)) * weight(r.x, r.y, x, y)) / sum_weight(x, y)));
        output(x, y, c) = bilateral(x, y, c);

        // schedule
        output.tile(x, y, xo, yo, xi, yi, 64, 64).fuse(xo, yo, outer).parallel(outer);
        kernel.store_root().compute_root();
        weight.store_at(output, outer).compute_at(output, yi);
        bilateral.store_at(output, outer).compute_at(output, yi);

        return output;
    }

    Func rgb_to_yuv(Func input) {                                                          
        Func output("rgb_to_yuv");

        Var x, y, c;

        Expr r = i16(input(x, y, 0));
        Expr g = i16(input(x, y, 1));
        Expr b = i16(input(x, y, 2));

        output(x, y, c) = select(
                c == 0, i16((38 * r + 75 * g + 15 * b) >> 7),
                c == 1, i16((-22 * r - 42 * g +  64 * b) >> 7),
                i16((64 * r - 54 * g - 10 * b) >> 7));

        // schedule
        output.compute_root().parallel(y).vectorize(x, 8);

        return output;
    }

    Func yuv_to_rgb(Func input) {

        Func output("yuv_to_rgb");

        Var x, y, c;

        Expr Y = i16(input(x, y, 0));
        Expr U = i16(input(x, y, 1));
        Expr V = i16(input(x, y, 2));

        output(x, y, c) = select(
                c == 0, u8_sat((128 * Y + 180 * V) >> 7),
                c == 1, u8_sat((128 * Y -  44 * U - 91 * V) >> 7),
                u8_sat((128 * Y + 227 * U) >> 7));

        // schedule

        output.compute_root().parallel(y).vectorize(x, 8);

        return output;
    }

};
}
HALIDE_REGISTER_GENERATOR(Finish, finish)
