#include "Halide.h"

namespace {

using namespace Halide::ConciseCasts;

class Motion : public Halide::Generator<Motion> {
public:
    Input<Buffer<int16_t>>  input{"input", 4};
    Output<Buffer<uint8_t>> output{"output", 4};

    void generate() {
        // Algorithm
        Func HSV = motion_to_HSV(Func(input));
        Func RGB = HSV_to_RGB(HSV);
        
        output(tx, ty, c, n) = RGB(tx, ty, c, n);

        // Schedule
        Var yo("yo"), yi("yi");
        output.reorder(c, tx, ty, n).split(ty, yo, yi, 8).parallel(yo).vectorize(tx, 8);
        HSV.store_at(output, yo).compute_at(output, yi).vectorize(tx, 8);
    }

private:
    Var tx, ty, c, n;
    
    Func motion_to_HSV(Func motion) {
        /* TODO
         * In order to map to hardware, need to change 'atan2' and 'sqrt' function
         * to LUT in the future.
         */

        Expr angle;
        Expr magnitude;
        Func HSV, output;

        // Need to change to integer or fixed point operation.
        angle = atan2(motion(tx, ty, 1, n), motion(tx, ty, 0, n)) * 180.0f / 3.141592f;
        magnitude = clamp(sqrt(f32(motion(tx, ty, 0, n)) * f32(motion(tx, ty, 0, n))  + f32(motion(tx, ty, 1, n)) * f32(motion(tx, ty, 1, n))), 0.0f, 128.0f);

        Expr scale = Expr(1.0f / 128.0f);
        HSV(tx, ty, c, n) = select(c == 0, angle, c == 1, magnitude * scale, Expr(1.0f));
        
        output(tx, ty, c, n) = HSV(tx, ty, c, n);

        return output;
    }

    Func HSV_to_RGB(Func HSV) {
        /*
         * In order to map to hardware, need to change algorithm with fixed point.
         */

        Func RGB, output;
        Expr r, g, b;
        Expr h, s, v;
        h = HSV(tx, ty, 0, n);
        s = HSV(tx, ty, 1, n);
        v = HSV(tx, ty, 2, n);
        Expr hd, i, f, p, q, t;

        hd = h / Expr(60.0f);
        i = floor(hd);
        f = hd - i;
        p = v * (Expr(1.0f) - s);
        q = v * (Expr(1.0f) - (s * f));
        t = v * (Expr(1.0f) - (s * (Expr(1.0f) - f)));

        r = select(
                i == 0, v,
                i == 1, q,
                i == 2, p,
                i == 3, p,
                i == 4, t,
                v);
        g = select(
                i == 0, t,
                i == 1, v,
                i == 2, v,
                i == 3, q,
                i == 4, p,
                p);
        b = select(
                i == 0, p,
                i == 1, p,
                i == 2, t,
                i == 3, v,
                i == 4, v,
                q);

        RGB(tx, ty, c, n) = select(c == 0, r, c == 1, g, b);
        output(tx, ty, c, n) = u8(RGB(tx, ty, c, n) * 255.0f);

        return output;
    }
};
}

HALIDE_REGISTER_GENERATOR(Motion, motion)
