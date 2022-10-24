#include "Halide.h"

#include <algorithm>

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;
Var x, y, z, c, xi, yi, xo, yo;

class LensBlur : public Halide::Generator<LensBlur> {
public:
    Input<Buffer<uint8_t>> input{"input", 3};
    Output<Buffer<uint8_t>> final{"final",3};
    // The number of displacements to consider
    const int slices = 32;
    // The depth to focus on
    const int focus_depth = 13;
    // The increase in blur radius with misfocus depth
    const float blur_radius_scale = 0.5f; //0.0f to 1.0f
    // The number of samples of the aperture to use
    const int aperture_samples = 32;

    void generate() {
        /* THE ALGORITHM */
        Expr maximum_blur_radius =
            cast<int>(max(cast<float>(slices) - focus_depth, focus_depth) * blur_radius_scale);
            

        Func hw_input;

        hw_input = BoundaryConditions::repeat_edge(input);

        Func diff;
        diff(x, y, z, c) = min(absd(hw_input(x, y, c), hw_input(x + 2 * z, y, c)),
                               absd(hw_input(x, y, c), hw_input(x + 2 * z + 1, y, c)));

        Func cost;
        cost(x, y, z) = (pow(cast<float>(diff(x, y, z, 0)), 2) +
                         pow(cast<float>(diff(x, y, z, 1)), 2) +
                         pow(cast<float>(diff(x, y, z, 2)), 2));

        // Compute confidence of cost estimate at each pixel by taking the
        // variance across the stack.
        Func cost_confidence;    

        RDom r(0, slices);
        Expr a = sum(pow(cost(x, y, r), 2)) / slices;
        Expr b = pow(sum(cost(x, y, r) / slices), 2);
        cost_confidence(x, y) = a - b;

        // Do a push-pull thing to blur the cost volume with an
        // exponential-decay type thing to inpaint over regions with low
        // confidence.
        Func cost_pyramid_push[8];
        cost_pyramid_push[0](x, y, z, c) =
            select(c == 0, cost(x, y, z) * cost_confidence(x, y), 
                    cost_confidence(x, y));

        Expr w = input.dim(0).extent(), h = input.dim(1).extent();
        for (int i = 1; i < 8; i++) {
            cost_pyramid_push[i](x, y, z, c) = downsample(cost_pyramid_push[i - 1])(x, y, z, c);
            w >> 1;
            h >> 1;
            cost_pyramid_push[i] = BoundaryConditions::repeat_edge(cost_pyramid_push[i], {{0, w}, {0, h}});
        }

        Func cost_pyramid_pull[8];
        cost_pyramid_pull[7](x, y, z, c) = cost_pyramid_push[7](x, y, z, c);
        for (int i = 6; i >= 0; i--) {
            cost_pyramid_pull[i](x, y, z, c) = lerp(upsample(cost_pyramid_pull[i + 1])(x, y, z, c),
                                                    cost_pyramid_push[i](x, y, z, c),
                                                    0.5f);
        }

        Func filtered_cost;
        filtered_cost(x, y, z) = (cost_pyramid_pull[0](x, y, z, 0) /
                                  cost_pyramid_pull[0](x, y, z, 1));

        // Assume the minimum cost slice is the correct depth.
        Func depth;

        RDom r2(0, slices);
        depth(x, y) = argmin(filtered_cost(x, y, r2))[0];


        Func bokeh_radius;
        bokeh_radius(x, y) = abs(depth(x, y) - focus_depth) * blur_radius_scale;

        Func bokeh_radius_squared;
        bokeh_radius_squared(x, y) = pow(bokeh_radius(x, y), 2);

        // Take a max filter of the bokeh radius to determine the
        // worst-case bokeh radius to consider at each pixel. Makes the
        // sampling more efficient below.
        Func worst_case_bokeh_radius_y;
        Func worst_case_bokeh_radius;

        RDom r3(-maximum_blur_radius, 2 * maximum_blur_radius + 1);
        worst_case_bokeh_radius_y(x, y) = maximum(bokeh_radius(x, y + r3));
        worst_case_bokeh_radius(x, y) = maximum(worst_case_bokeh_radius_y(x + r3, y));


        Func input_with_alpha;
        /*input_with_alpha(x, y, c) = select(c == 0, cast<float>(hw_input(x, y, 0)),
                                           c == 1, cast<float>(hw_input(x, y, 1)),
                                           c == 2, cast<float>(hw_input(x, y, 2)),
                                            255.0f);
        */
        input_with_alpha(x, y, c) = select(c == 0, cast<uint16_t>(hw_input(x, y, 0)),
                                           c == 1, cast<uint16_t>(hw_input(x, y, 1)),
                                           c == 2, cast<uint16_t>(hw_input(x, y, 2)),
                                            255);
        // Render a blurred image
        Func output;
        output(x, y, c) = input_with_alpha(x, y, c);

        // The sample locations are a random function of x, y, and sample
        // number (not c).
        Expr worst_radius = worst_case_bokeh_radius(x, y);
        Expr sample_u = (random_float() - 0.5f) * 2 * worst_radius;
        Expr sample_v = (random_float() - 0.5f) * 2 * worst_radius;
        sample_u = clamp(cast<int>(sample_u), -maximum_blur_radius, maximum_blur_radius);
        sample_v = clamp(cast<int>(sample_v), -maximum_blur_radius, maximum_blur_radius);
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
                   u16(1), 0);

        sample_x = x + sample_locations(x, y, s)[0];
        sample_y = y + sample_locations(x, y, s)[1];
        output(x, y, c) += cast<uint16_t>(sample_weight(x, y, s) * input_with_alpha(sample_x, sample_y, c));

        Func output_norm;
        output_norm(x, y, c) = cast<uint8_t>(output(x, y, c) >> 8); //normalize norm values to fit in 8-bit

        Func rom_div_lookup;
        rom_div_lookup(x) = u16( (u32(1) << 8) / u32(x)); //8 to 8.8 format

        Func reciprocal;
        reciprocal(x, y) = rom_div_lookup(max(output_norm(x, y, 3), u16(1))); //8.8 format

        Func hw_output;
        hw_output(x, y, c) = cast<uint8_t>(clamp(output_norm(x, y, c) * reciprocal(x, y), 0, 255)); 

        //hw_output(x, y, c) = cast<uint8_t>(255.0f*output(x, y, c) / output(x, y, 3));
        final(x, y, c) = hw_output(x, y, c);

        final.bound(c, 0, 3);
        /* THE SCHEDULE */

        if (get_target().has_feature(Target::Clockwork)) {
            // CGRA Schedule
            const int numTiles = 1;
            const int tileSize = 128;
            const int glbSize = tileSize*numTiles;

            const int unroll = 3;
            
            hw_output.in().compute_root();

            hw_output.in()
                .tile(x, y, xo, yo, xi, yi, glbSize, glbSize)
                .reorder(c, xi, yi, xo, yo)
                .hw_accelerate(xi, xo)
                .unroll(c)
                .unroll(xi, unroll, TailStrategy::RoundUp)
                .store_in(MemoryType::GLB);

            hw_output
                .tile(x, y, xo, yo, xi, yi, tileSize, tileSize)
                .reorder(c, xi, yi, xo, yo)
                .compute_at(hw_output.in(), xo)
                .unroll(c)
                .unroll(xi, unroll, TailStrategy::RoundUp);
            
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
                //.unroll(z)
                .unroll(x, unroll, TailStrategy::RoundUp);

            sample_locations
                .reorder(z, x, y)
                .compute_at(hw_output, xo)
                //.unroll(z)
                .unroll(x, unroll, TailStrategy::RoundUp);

            input_with_alpha
                .reorder(c, x, y)
                .compute_at(hw_output, xo)
                .unroll(c)
                .unroll(x, unroll, TailStrategy::RoundUp);
            
            worst_case_bokeh_radius
                .compute_at(hw_output, xo)
                //.unroll(r2)
                .unroll(x, unroll, TailStrategy::RoundUp);
            
            worst_case_bokeh_radius_y
                .compute_at(hw_output, xo)
                //.unroll(r2)
                .unroll(x, unroll, TailStrategy::RoundUp);

            bokeh_radius_squared
                .compute_at(hw_output, xo)
                .unroll(x, unroll, TailStrategy::RoundUp);

            bokeh_radius
                .compute_at(hw_output, xo)
                .unroll(x, unroll, TailStrategy::RoundUp);

            depth
                .compute_at(hw_output, xo)
                //.unroll(r1)
                .unroll(x, unroll, TailStrategy::RoundUp);

            filtered_cost
                .reorder(z, x, y)
                .compute_at(hw_output, xo)
                //.unroll(z)
                .unroll(x, unroll, TailStrategy::RoundUp);

            for (int i = 0; i < 8; i++) {
                cost_pyramid_push[i].compute_at(hw_output, xo)
                    .reorder(c, z, x, y)
                    .unroll(c)
                    //.unroll(z)
                    .unroll(x, unroll, TailStrategy::RoundUp);

                cost_pyramid_pull[i]
                    .compute_at(hw_output, xo)
                    .reorder(c, z, x, y)
                    .unroll(c)
                    //.unroll(z)
                    .unroll(x, unroll, TailStrategy::RoundUp);
            }            
            
            cost_confidence
                .compute_at(hw_output, xo)
                //.unroll(r)
                .unroll(x, unroll, TailStrategy::RoundUp);

            cost
                .reorder(z, x, y)
                .compute_at(hw_output, xo)
                //.unroll(z)
                .unroll(x, unroll, TailStrategy::RoundUp);

            diff
                .reorder(c, z, x, y)
                .compute_at(hw_output, xo)
                .unroll(c)
                //.unroll(z)
                .unroll(x, unroll, TailStrategy::RoundUp);

            hw_input.in().in().compute_at(hw_output, xo); // represents the mem tile
            hw_input.in().in().unroll(_0, unroll, TailStrategy::RoundUp);

            hw_input.in().compute_at(hw_output.in(), xo); // represents GLB level
            hw_input.in().store_in(MemoryType::GLB);
            hw_input.in().unroll(_0, unroll, TailStrategy::RoundUp);
            
            hw_input.compute_root()
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
            sample_locations.compute_at(output, x).vectorize(x);
        }
    }

private:
    Var x, y, z, c;

    // Downsample with a 1 3 3 1 filter
    Func downsample(Func f) {
        using Halide::_;
        Func downx, downy;
        downx(x, y, _) = ((f(2 * x - 1, y, _) + 3.0f * (f(2 * x, y, _) + f(2 * x + 1, y, _)) + f(2 * x + 2, y, _)))/8.0f;
        downy(x, y, _) = ((downx(x, 2 * y - 1, _) + 3.0f * (downx(x, 2 * y, _) + downx(x, 2 * y + 1, _)) + downx(x, 2 * y + 2, _)))/8.0f;
        return downy;
    }

    // Upsample using bilinear interpolation
    Func upsample(Func f) {
        using Halide::_;
        Func upx, upy;
        upx(x, y, _) = 0.25f * f((x / 2) - 1 + 2 * (x % 2), y, _) + 0.75f * f(x / 2, y, _);
        upy(x, y, _) = 0.25f * upx(x, (y / 2) - 1 + 2 * (y % 2), _) + 0.75f * upx(x, y / 2, _);
        return upy;
    }

};

}  // namespace

HALIDE_REGISTER_GENERATOR(LensBlur, lensblur)

/*
Found an xcel call: _hls_target.hw_output_global_wrapper
Found an accelerator: _hls_accelerator.hw_output_global_wrapper
rom_div_lookup is a realization type 3

Program received signal SIGSEGV, Segmentation fault.
0x0000559305038df4 in Halide::Internal::IRMutator::visit(Halide::Internal::Add const*) ()

#0  0x000055951ea9335e in Halide::Internal::ExprNode<Halide::Internal::IntImm>::mutate_expr(Halide::Internal::IRMutator*) const ()
#1  0x000055951eaa2074 in Halide::Internal::IRMutator::mutate(Halide::Expr const&) ()
#2  0x000055951eaa2df7 in Halide::Internal::IRMutator::visit(Halide::Internal::Add const*) ()
#3  0x000055951ea8a53f in Halide::Internal::ExprNode<Halide::Internal::Add>::mutate_expr(Halide::Internal::IRMutator*) const ()
#4  0x000055951eaa2074 in Halide::Internal::IRMutator::mutate(Halide::Expr const&) ()
#5  0x000055951eaa3772 in Halide::Internal::IRMutator::visit(Halide::Internal::Cast const*) ()
#6  0x000055951ea8a49f in Halide::Internal::ExprNode<Halide::Internal::Cast>::mutate_expr(Halide::Internal::IRMutator*) const ()
#7  0x000055951eaa2074 in Halide::Internal::IRMutator::mutate(Halide::Expr const&) ()
#8  0x000055951eaa3499 in Halide::Internal::IRMutator::visit(Halide::Internal::Div const*) ()
#9  0x000055951ea8a62f in Halide::Internal::ExprNode<Halide::Internal::Div>::mutate_expr(Halide::Internal::IRMutator*) const ()
#10 0x000055951eaa2074 in Halide::Internal::IRMutator::mutate(Halide::Expr const&) ()
#11 0x000055951eaa4f2e in Halide::Internal::IRMutator::visit(Halide::Internal::Call const*) ()
#12 0x000055951f568a30 in Halide::Internal::InlineMemoryConstants::visit(Halide::Internal::Call const*) ()
#13 0x000055951ea8ac62 in Halide::Internal::ExprNode<Halide::Internal::Call>::mutate_expr(Halide::Internal::IRMutator*) const ()
#14 0x000055951eaa2074 in Halide::Internal::IRMutator::mutate(Halide::Expr const&) ()
#15 0x000055951eaa3772 in Halide::Internal::IRMutator::visit(Halide::Internal::Cast const*) ()
#16 0x000055951ea8a49f in Halide::Internal::ExprNode<Halide::Internal::Cast>::mutate_expr(Halide::Internal::IRMutator*) const ()
#17 0x000055951eaa2074 in Halide::Internal::IRMutator::mutate(Halide::Expr const&) ()
#130822 0x000055951f5666bb in Halide::Internal::inline_memory_constants(Halide::Internal::Stmt) ()
#130823 0x000055951ef57cf3 in Halide::Internal::lower(std::vector<Halide::Internal::Function, std::allocator<Halide::Internal::Function> > const&, std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const&, Halide::Target const&, std::vector<Halide::Argument, std::allocator<Halide::Argument> > const&, Halide::LinkageType, std::vector<Halide::Internal::IRMutator*, std::allocator<Halide::Internal::IRMutator*> > const&) ()
#130824 0x000055951eae6c0e in Halide::Pipeline::compile_to_module(std::vector<Halide::Argument, std::allocator<Halide::Argument> > const&, std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const&, Halide::Target const&, Halide::LinkageType) ()
#130825 0x000055951ea3bed3 in Halide::Internal::GeneratorBase::build_module(std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const&, Halide::LinkageType) ()
#130826 0x000055951ea3c9f8 in Halide::Internal::generate_filter_main(int, char**, std::ostream&)::{lambda(std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const&, Halide::Target const&)#1}::operator()(std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const&, Halide::Target const&) const [clone .isra.0] ()
#130827 0x000055951ea3e6c9 in Halide::Internal::generate_filter_main(int, char**, std::ostream&) ()
#130828 0x00005595209952d4 in main (argc=10, argv=0x7fff4048e9e8) at /aha/Halide-to-Hardware/distrib/tools/GenGen.cpp:4
*/