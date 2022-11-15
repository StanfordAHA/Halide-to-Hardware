#include "Halide.h"

#include <algorithm>

namespace {


const int width = 192;
const int height = 320;

using namespace Halide;
using namespace Halide::ConciseCasts;
Var x, y, z, c, xi, yi, xo, yo;

class LensBlur : public Halide::Generator<LensBlur> {
public:
    Input<Buffer<uint8_t>> input{"input", 3};
    Output<Buffer<uint8_t>> final{"final",3};
    // The number of displacements to consider
    const uint8_t slices = 32;
    // The depth to focus on
    const uint8_t focus_depth = 13;
    // The increase in blur radius with misfocus depth
    const uint8_t blur_radius_scale = 1; //0.0f to 1.0f
    // The number of samples of the aperture to use
    const uint8_t aperture_samples = 32;

    void generate() {
        /* THE ALGORITHM */
        Expr maximum_blur_radius =
            cast<int16_t>(max(cast<uint16_t>(slices) - focus_depth, focus_depth) >> 1);

        Func rom_div_lookup;
        rom_div_lookup(x) = u16( (u32(1) << 8) / u32(x)); 

        Func input_bound, hw_input;

        input_bound = BoundaryConditions::repeat_edge(input);
        hw_input(x, y, c) = input_bound(x, y, c);

        Func diff;
        diff(x, y, z, c) = min(absd(hw_input(x, y, c), hw_input(x + 2 * z, y, c)),
                               absd(hw_input(x, y, c), hw_input(x + 2 * z + 1, y, c)));

        Func cost;
        cost(x, y, z) = (diff(x, y, z, 0) * diff(x, y, z, 0) +
                         diff(x, y, z, 1) * diff(x, y, z, 1) +
                         diff(x, y, z, 2) * diff(x, y, z, 2)); //Normalized to fit in 8-bit

        // Compute confidence of cost estimate at each pixel by taking the
        // variance across the stack.
        Func cost_confidence;    

        RDom r(0, slices);
        Expr a = sum((cost(x, y, r) * cost(x, y, r))) >> 5; //slices = 32, shift >> 5 instead
        Expr b = (sum((cost(x, y, r)) >> 5) * sum((cost(x, y, r)) >> 5)); //slices = 32 , shift >> 5 isntead
        cost_confidence(x, y) = cast<uint16_t>(a - b);

        // Do a push-pull thing to blur the cost volume with an
        // exponential-decay type thing to inpaint over regions with low
        // confidence.
        Func cost_pyramid_push[8];
        cost_pyramid_push[0](x, y, z, c) =
            select(c == 0, (clamp((cost(x, y, z) * cost_confidence(x, y)), 0, 65535)) >> 8, 
                    cost_confidence(x, y));

        Expr w = input.dim(0).extent(), h = input.dim(1).extent();
        for (int i = 1; i < 8; i++) {
            /*
            Expr print_cost_pyr = downsample(cost_pyramid_push[i - 1])(x, y, z, c) >> 8;
            print_cost_pyr = print_when(print_cost_pyr > 30000, print_cost_pyr, "<- cost pyramid push[",i,"] at x = ",x,", y =",y);
            */
            cost_pyramid_push[i](x, y, z, c) = cast<uint16_t>(downsample(cost_pyramid_push[i - 1])(x, y, z, c)) >> 8;
            w >> 1;
            h >> 1;
            cost_pyramid_push[i] = BoundaryConditions::repeat_edge(cost_pyramid_push[i], {{0, w}, {0, h}});
        }
        
        Func cost_pyramid_pull[8];
        cost_pyramid_pull[7](x, y, z, c) = cost_pyramid_push[7](x, y, z, c);
        for (int i = 6; i >= 0; i--) {
            cost_pyramid_pull[i](x, y, z, c) = cast<uint16_t>(upsample(cost_pyramid_pull[i + 1])(x, y, z, c) + ((cost_pyramid_push[i](x, y, z, c) - upsample(cost_pyramid_pull[i + 1])(x, y, z, c)) >> 1)) >> 8;;
        }

        //lerp = a + t(b - a)

        Func filtered_cost_reciprocal;

        filtered_cost_reciprocal(x, y, z) = rom_div_lookup(max(cost_pyramid_pull[0](x, y, z, 1), u16(1))); //8.8 format

        Func filtered_cost;
        filtered_cost(x, y, z) = (cost_pyramid_pull[0](x, y, z, 0) * filtered_cost_reciprocal(x, y, z)) >> 8;

        // Assume the minimum cost slice is the correct depth.
        Func depth;

        RDom r2(0, slices);  

        //depth(x, y) = argmin(filtered_cost(x, y, r2))[0]
        Func filtered_cost_min;

        filtered_cost_min(x, y) = minimum(filtered_cost(x, y, r2));

        depth(x, y) = cast<int8_t>(0);
        depth(x, y) = select(filtered_cost(x, y, r2) == filtered_cost_min(x, y),
                                cast<int8_t>(r2),
                                depth(x, y));

        Func bokeh_radius;
        bokeh_radius(x, y) = cast<uint16_t>(abs(depth(x, y) - focus_depth)) >> 1;

        Func bokeh_radius_squared;
        bokeh_radius_squared(x, y) = bokeh_radius(x, y) * bokeh_radius(x, y);

        // Take a max filter of the bokeh radius to determine the
        // worst-case bokeh radius to consider at each pixel. Makes the
        // sampling more efficient below.
        Func worst_case_bokeh_radius_y;
        Func worst_case_bokeh_radius;

        RDom r3(-maximum_blur_radius, 2 * maximum_blur_radius + 1);
        worst_case_bokeh_radius_y(x, y) = maximum(bokeh_radius(x, y + r3));
        worst_case_bokeh_radius(x, y) = maximum(worst_case_bokeh_radius_y(x + r3, y));

        Func input_with_alpha;
        input_with_alpha(x, y, c) = select(c == 0, cast<uint16_t>(hw_input(x, y, 0)),
                                           c == 1, cast<uint16_t>(hw_input(x, y, 1)),
                                           c == 2, cast<uint16_t>(hw_input(x, y, 2)),
                                           cast<uint16_t>(255));
        // Render a blurred image
        Func output;
        output(x, y, c) = input_with_alpha(x, y, c);

        // The sample locations are a random function of x, y, and sample
        // number (not c).

        Func random_v, random_u, hw_ru, hw_rv;

        Expr random = cast<uint16_t>(clamp(random_float()*255, 0, 255));
        //Expr random = cast<uint16_t>(clamp(random_int() >> 24, 0, 255));
        random_u(x, y, z) = random;
        random_v(x, y, z) = random;

        hw_ru(x, y, z) = random_u(x, y, z);
        hw_rv(x, y, z) = random_v(x, y, z);

        Expr worst_radius = worst_case_bokeh_radius(x, y);
        
        Expr sample_u = cast<int16_t>((hw_ru(x, y, z) - cast<uint16_t>(128)) * cast<uint16_t>(worst_radius)) >> 7;
        Expr sample_v = cast<int16_t>((hw_rv(x, y, z) - cast<uint16_t>(128)) * cast<uint16_t>(worst_radius)) >> 7;

        sample_u = clamp(cast<int16_t>(sample_u), -maximum_blur_radius, maximum_blur_radius);
        sample_v = clamp(cast<int16_t>(sample_v), -maximum_blur_radius, maximum_blur_radius);
        Func sample_locations;
        sample_locations(x, y, z) = {sample_u, sample_v};

        RDom s(0, aperture_samples);
        sample_u = sample_locations(x, y, z)[0];
        sample_v = sample_locations(x, y, z)[1];
        Expr sample_x = x + sample_u, sample_y = y + sample_v;
        Expr r_squared = sample_u * sample_u + sample_v * sample_v;

        // We use this sample if it's from a pixel whose bokeh influences
        // this output pixel. Here's a crude approximation that ignores
        // some subtleties of occlusion edges and inpaints behind objects.
        Expr sample_is_within_bokeh_of_this_pixel =
            r_squared < bokeh_radius_squared(x, y);

        Expr this_pixel_is_within_bokeh_of_sample =
            r_squared < bokeh_radius_squared(sample_x, sample_y);

        Expr sample_is_in_front_of_this_pixel =
            depth(sample_x, sample_y) < depth(x, y);

        Func sample_weight;
        sample_weight(x, y, z) =
            select((sample_is_within_bokeh_of_this_pixel ||
                    sample_is_in_front_of_this_pixel) &&
                       this_pixel_is_within_bokeh_of_sample,
                   u16(1), u16(0));

        sample_x = x + sample_locations(x, y, s)[0];
        sample_y = y + sample_locations(x, y, s)[1];

        output(x, y, c) += cast<uint16_t>(sample_weight(x, y, s) * input_with_alpha(sample_x, sample_y, c));

        Func output_norm;
        output_norm(x, y, c) = cast<uint8_t>(output(x, y, c) >> 8); //normalize norm values to fit in 8-bit

        Func reciprocal;
        reciprocal(x, y) = rom_div_lookup(max(output_norm(x, y, 3), u16(1))); //8.8 format

        Func hw_output;
        hw_output(x, y, c) = cast<uint8_t>(clamp(output_norm(x, y, c) * reciprocal(x, y), 0, 255)); 

        //hw_output(x, y, c) = cast<uint8_t>(255.0f*output(x, y, c) / output(x, y, 3));
        final(x, y, c) = hw_output(x, y, c);

        /* THE SCHEDULE */
        final.bound(c, 0, 3);
        final.bound(x, 0, 192);
        final.bound(y, 0, 320);
        
        if (get_target().has_feature(Target::Clockwork)) {
            // CGRA Schedule
            const int numTiles = 1;
            const int tileSize = 32;
            const int glbSize = 64;

            const int unroll = 3;

            hw_output.in()
                .compute_root()
                .tile(x, y, xo, yo, xi, yi, glbSize, glbSize)
                .reorder(c, xi, yi, xo, yo)
                .hw_accelerate(xi, xo)
                .unroll(c)
                .unroll(xi, unroll, TailStrategy::RoundUp);

            hw_output
                .tile(x, y, xo, yo, xi, yi, tileSize, tileSize)
                .reorder(c, xi, yi, xo, yo)
                .compute_at(hw_output.in(), xo)
                .unroll(c)
                .unroll(xi, unroll, TailStrategy::RoundUp)
                .store_in(MemoryType::GLB);
            
            reciprocal
                .compute_at(hw_output, xo)
                .unroll(x, unroll, TailStrategy::RoundUp);

            output_norm
                .compute_at(hw_output, xo)
                .unroll(c)
                .unroll(x, unroll, TailStrategy::RoundUp);

            output.update()
                .unroll(s, aperture_samples)
                .unroll(c)
                .unroll(x, unroll, TailStrategy::RoundUp);

            output
                .compute_at(hw_output, xo)
                .unroll(c)
                .unroll(x, unroll, TailStrategy::RoundUp);
            
            rom_div_lookup
                .compute_at(hw_output, xo)
                .unroll(x); // look up in 8.8 format at ROM Memory

            sample_weight
                .reorder(z, x, y)
                .compute_at(hw_output, xo)
                .unroll(x, unroll, TailStrategy::RoundUp);

            input_with_alpha
                .reorder(c, x, y)
                .compute_at(hw_output, xo)
                .unroll(c)
                .unroll(x, unroll, TailStrategy::RoundUp);
            
            worst_case_bokeh_radius
                .compute_at(hw_output, xo)
                .unroll(x, unroll, TailStrategy::RoundUp);
            
            worst_case_bokeh_radius_y
                .compute_at(hw_output, xo)
                .unroll(x, unroll, TailStrategy::RoundUp);

            bokeh_radius_squared
                .compute_at(hw_output, xo)
                .unroll(x, unroll, TailStrategy::RoundUp);

            bokeh_radius
                .compute_at(hw_output, xo)
                .unroll(x, unroll, TailStrategy::RoundUp);
            
            depth.update()
                .unroll(x, unroll, TailStrategy::RoundUp);
            
            depth
                .compute_at(hw_output, xo)
                .unroll(x, unroll, TailStrategy::RoundUp);

            filtered_cost
                .reorder(z, x, y)
                .compute_at(hw_output, xo)
                .unroll(x, unroll, TailStrategy::RoundUp);
            
            filtered_cost_min
                .compute_at(hw_output, xo)
                .unroll(x, unroll, TailStrategy::RoundUp);

            filtered_cost_reciprocal
                .compute_at(hw_output, xo)
                .unroll(x, unroll, TailStrategy::RoundUp);
            
            for (int i = 0; i < 8; i++) {
                cost_pyramid_push[i].compute_at(hw_output, xo)
                    .reorder(c, z, x, y)
                    .unroll(c)
                    .unroll(x, unroll, TailStrategy::RoundUp);

                cost_pyramid_pull[i]
                    .compute_at(hw_output, xo)
                    .reorder(c, z, x, y)
                    .unroll(c)
                    .unroll(x, unroll, TailStrategy::RoundUp);
            }               
            cost_confidence
                .compute_at(hw_output, xo)
                .unroll(x, unroll, TailStrategy::RoundUp);
            cost
                .reorder(z, x, y)
                .compute_at(hw_output, xo)
                .unroll(x, unroll, TailStrategy::RoundUp);
            diff
                .reorder(c, z, x, y)
                .compute_at(hw_output, xo)
                .unroll(c)
                .unroll(x, unroll, TailStrategy::RoundUp);
            
            hw_rv.in().in()
                .compute_at(hw_output, xo)
                .unroll(x, unroll, TailStrategy::RoundUp);

            hw_ru.in().in()
                .compute_at(hw_output, xo)
                .unroll(x, unroll, TailStrategy::RoundUp);
            
            hw_rv.in()
                .compute_at(hw_output.in(), xo)
                .store_in(MemoryType::GLB)
                .unroll(x, unroll, TailStrategy::RoundUp);

            hw_ru.in()
                .compute_at(hw_output.in(), xo)
                .store_in(MemoryType::GLB)
                .unroll(x, unroll, TailStrategy::RoundUp);
            
            hw_rv
                .compute_root()
                .accelerator_input();
            
            hw_ru
                .compute_root()
                .accelerator_input();
            
            hw_input.in().in()
                .compute_at(hw_output, xo) // represents the mem tile
                .unroll(x, unroll, TailStrategy::RoundUp);

            hw_input.in()
                .compute_at(hw_output.in(), xo) // represents GLB level
                .store_in(MemoryType::GLB)
                .unroll(x, unroll, TailStrategy::RoundUp);
            
            hw_input
              .compute_root()
              .accelerator_input(); 
            
        } else {
            // Manual CPU schedule
            //
            cost_pyramid_push[0]
                .compute_root()
                .reorder(c, z, x, y)
                .bound(c, 0, 2)
                .unroll(c)
                .vectorize(x, 16)
                .parallel(y, 4);
            cost.compute_at(cost_pyramid_push[0], x)
                .vectorize(x);
            cost_confidence.compute_at(cost_pyramid_push[0], x)
                .vectorize(x);

            Var xi, yi, t;
            for (int i = 1; i < 8; i++) {
                cost_pyramid_push[i].compute_at(cost_pyramid_pull[1], t).vectorize(x, 8);
                if (i > 1) {
                    cost_pyramid_pull[i]
                        .compute_at(cost_pyramid_pull[1], t)
                        .tile(x, y, xi, yi, 8, 2)
                        .vectorize(xi)
                        .unroll(yi);
                }
            }

            cost_pyramid_pull[1]
                .compute_root()
                .fuse(z, c, t)
                .parallel(t)
                .tile(x, y, xi, yi, 8, 2)
                .vectorize(xi)
                .unroll(yi);
            depth.compute_root()
                .tile(x, y, xi, yi, 8, 2)
                .vectorize(xi)
                .unroll(yi)
                .parallel(y, 8);
            input_with_alpha.compute_root()
                .reorder(c, x, y)
                .unroll(c)
                .vectorize(x, 8)
                .parallel(y, 8);
            worst_case_bokeh_radius_y
                .compute_at(final, y)
                .vectorize(x, 8);
            final.compute_root()
                .reorder(c, x, y)
                .bound(c, 0, 3)
                .unroll(c)
                .vectorize(x, 8)
                .parallel(y);
            worst_case_bokeh_radius
                .compute_at(final, y)
                .vectorize(x, 8);
            output.compute_at(final, x)
                .vectorize(x);
            output.update()
                .reorder(c, x, s)
                .vectorize(x)
                .unroll(c);
            sample_weight.compute_at(output, x).unroll(x);
            /*
            sample_locations_y.compute_at(output, x).vectorize(x);
            sample_locations_x.compute_at(output, x).vectorize(x);
            */
        }
    }

private:
    Var x, y, z, c;

    // Downsample with a 1 3 3 1 filter
    Func downsample(Func f) {
        using Halide::_;
        Func downx, downy;
        downx(x, y, _) = ((f(2 * x - 1, y, _) + 3 * (f(2 * x, y, _) + f(2 * x + 1, y, _)) + f(2 * x + 2, y, _))) >> 3;
        downy(x, y, _) = ((downx(x, 2 * y - 1, _) + 3 * (downx(x, 2 * y, _) + downx(x, 2 * y + 1, _)) + downx(x, 2 * y + 2, _))) >> 3;
        return downy;
    }

    // Upsample using bilinear interpolation
    Func upsample(Func f) {
        using Halide::_;
        Func upx, upy;
        upx(x, y, _) = (f((x / 2) - 1 + 2 * (x % 2), y, _) + 3 * f(x / 2, y, _)) >> 2;
        upy(x, y, _) = (upx(x, (y / 2) - 1 + 2 * (y % 2), _) +  3 * upx(x, y / 2, _)) >> 2;
        return upy;
    }

};

}  // namespace

HALIDE_REGISTER_GENERATOR(LensBlur, lensblur)
