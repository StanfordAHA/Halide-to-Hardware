#include "Halide.h"
#include "halide_trace_config.h"

namespace {
int s_sigma = 8;
int r_sigma = 16;

class BilateralGrid : public Halide::Generator<BilateralGrid> {
public:

    Input<Buffer<uint8_t>>  input{"input", 2};
    Output<Buffer<uint8_t>> output{"output", 2};

    Expr linterp(Expr zero_val, Expr one_val, Expr weight, int scale) {
        Expr result;
        result = (zero_val + ((weight * (one_val - zero_val))/scale));
        return result;
    }

    void generate() {
        Var x("x"), y("y"), z("z"), c("c");
        Var xo("xo"), yo("yo"), xi("xi"), yi("yi");

        Func clamped = Halide::BoundaryConditions::repeat_edge(input);

        RDom r(0, s_sigma, 0, s_sigma);
        Expr val = cast<uint16_t>(clamped(x*s_sigma + r.x, y*s_sigma + r.y));
        val = cast<uint16_t>(clamp(val, 0, 255));

        Expr zi = cast<int>(val/r_sigma);

        Func histogram, histogram_count;
        histogram_count(x, y, z) = cast<uint16_t>(0);
        histogram_count(x, y, zi) += cast<uint16_t>(1);

        histogram(x, y, z) = cast<uint16_t>(0);
        histogram(x, y, zi) += cast<uint16_t>(val);
        
        Func hw_input_hist, input_copy_hist;
        hw_input_hist(x, y, z) = cast<int16_t>(histogram(x, y, z));
        input_copy_hist(x, y, z) = hw_input_hist(x, y, z);

        Func blurx("blurx"), blury("blury"), blurz("blurz");
        blurz(x, y, z) = (input_copy_hist(x, y, z-2) +
                          input_copy_hist(x, y, z-1)*4 +
                          input_copy_hist(x, y, z  )*6 +
                          input_copy_hist(x, y, z+1)*4 +
                          input_copy_hist(x, y, z+2));
        blurx(x, y, z) = (blurz(x-2, y, z) +
                          blurz(x-1, y, z)*4 +
                          blurz(x  , y, z)*6 +
                          blurz(x+1, y, z)*4 +
                          blurz(x+2, y, z));
        blury(x, y, z) = (blurx(x, y-2, z) +
                          blurx(x, y-1, z)*4 +
                          blurx(x, y  , z)*6 +
                          blurx(x, y+1, z)*4 +
                          blurx(x, y+2, z));

        Func hw_input_hist_count, input_copy_hist_count;
        hw_input_hist_count(x, y, z) = cast<int16_t>(histogram_count(x, y, z));
        input_copy_hist_count(x, y, z) = hw_input_hist_count(x, y, z);

        Func blur_count_x("blur_count_x"), blur_count_y("blur_count_y"), blur_count_z("blur_count_z");
        blur_count_z(x, y, z) = (input_copy_hist_count(x, y, z-2) +
                                 input_copy_hist_count(x, y, z-1)*4 +
                                 input_copy_hist_count(x, y, z  )*6 +
                                 input_copy_hist_count(x, y, z+1)*4 +
                                 input_copy_hist_count(x, y, z+2));
        blur_count_x(x, y, z) = (blur_count_z(x-2, y, z) +
                                 blur_count_z(x-1, y, z)*4 +
                                 blur_count_z(x  , y, z)*6 +
                                 blur_count_z(x+1, y, z)*4 +
                                 blur_count_z(x+2, y, z));
        blur_count_y(x, y, z) = (blur_count_x(x, y-2, z) +
                                 blur_count_x(x, y-1, z)*4 +
                                 blur_count_x(x, y  , z)*6 +
                                 blur_count_x(x, y+1, z)*4 +
                                 blur_count_x(x, y+2, z));
        

        Func hw_input, input_copy;
        hw_input(x, y) = cast<uint16_t>(clamped(x, y));
        input_copy(x, y) = hw_input(x, y);

        Expr val2 = input_copy(x, y);
        val2 = clamp(val2, 0, 255);

        //Expr xf = cast<int>(clamp(x % s_sigma, 0, s_sigma));
        //Expr yf = cast<int>(clamp(y % s_sigma, 0, s_sigma));
        //Expr zf = cast<int>(clamp(val2 % r_sigma, 0, r_sigma));
        //
        //Expr xii = cast<int>(clamp(x/s_sigma, 0, (128/s_sigma)+1));
        //Expr yii = cast<int>(clamp(y/s_sigma, 0, (128/s_sigma)+1));
        //Expr zi2 = cast<int>(clamp(val2/r_sigma, 0, (255/r_sigma)+1));

        Func xf, yf, zf;
        xf(x) = cast<int>(clamp(x % s_sigma, 0, s_sigma));
        yf(y) = cast<int>(clamp(y % s_sigma, 0, s_sigma));
        zf(x, y) = cast<int>(clamp(val2 % r_sigma, 0, r_sigma));

        Func xii, yii, zi2, xii_p1, yii_p1, zi2_p1;
        xii(x) = cast<int>(clamp(x/s_sigma, 0, (128/s_sigma)+1));
        yii(y) = cast<int>(clamp(y/s_sigma, 0, (128/s_sigma)+1));
        zi2(x, y) = cast<int>(clamp(val2/r_sigma, 0, (255/r_sigma)+1));        
        xii_p1(x) = xii(x) + 1;
        yii_p1(y) = yii(y) + 1;
        zi2_p1(x, y) = zi2(x, y) + 1;

        Expr x_i = xii(x);
        Expr xi_p1 = xii_p1(x);
        Expr y_i = yii(y);
        Expr yi_p1 = yii_p1(y);
        Expr z_i = zi2(x,y);
        Expr zi_p1 = zi2_p1(x, y);

        Func interpolated("interpolated");
        Func interp_count("interp_count");
        interpolated(x, y) = linterp(linterp(linterp(blury(x_i, y_i,   z_i),   blury(xi_p1, y_i,   z_i),   xf(x), s_sigma),
                                             linterp(blury(x_i, yi_p1, z_i),   blury(xi_p1, yi_p1, z_i),   xf(x), s_sigma), yf(y), s_sigma),
                                     linterp(linterp(blury(x_i, y_i,   zi_p1), blury(xi_p1, y_i,   zi_p1), xf(x), s_sigma),
                                             linterp(blury(x_i, yi_p1, zi_p1), blury(xi_p1, yi_p1, zi_p1), xf(x), s_sigma), yf(y), s_sigma), zf(x,y), r_sigma);
        
        interp_count(x, y) = linterp(linterp(linterp(blur_count_y(x_i, y_i,   z_i),   blur_count_y(xi_p1, y_i,   z_i),   xf(x), s_sigma),
                                             linterp(blur_count_y(x_i, yi_p1, z_i),   blur_count_y(xi_p1, yi_p1, z_i),   xf(x), s_sigma), yf(y), s_sigma),
                                     linterp(linterp(blur_count_y(x_i, y_i,   zi_p1), blur_count_y(xi_p1, y_i,   zi_p1), xf(x), s_sigma),
                                             linterp(blur_count_y(x_i, yi_p1, zi_p1), blur_count_y(xi_p1, yi_p1, zi_p1), xf(x), s_sigma), yf(y), s_sigma), zf(x,y), r_sigma);


        //blury.bound(x, 2, 14);
        //blury.bound(y, 2, 14);
        //blury.bound(z, 0, 10);

        // Normalize
        Func hw_output;
        hw_output(x, y) = cast<uint8_t>(interpolated(x, y) / interp_count(x, y));
        output(x, y) = hw_output(x, y);

        output.bound(x,0,128);
        output.bound(y,0,128);

        if (get_target().has_feature(Target::Clockwork) || get_target().has_feature(Target::Pono)) {
            hw_output.compute_root();
            hw_output
              .tile(x, y, xo, yo, xi, yi, 128, 128)
              .hw_accelerate(xi, xo);

            input_copy_hist.compute_at(hw_output, xo);
            hw_input_hist.stream_to_accelerator();

            input_copy_hist_count.compute_at(hw_output, xo);
            hw_input_hist_count.stream_to_accelerator();


            interpolated.compute_at(hw_output, xo).store_at(hw_output, xo);

            xii_p1.compute_at(hw_output, xo);
            yii_p1.compute_at(hw_output, xo);
            zi2_p1.compute_at(hw_output, xo);
            xii.compute_at(hw_output, xo);
            yii.compute_at(hw_output, xo);
            zi2.compute_at(hw_output, xo);
            xf.compute_at(hw_output, xo);
            yf.compute_at(hw_output, xo);
            zf.compute_at(hw_output, xo);
            
            blurx.compute_at(hw_output, xo).store_at(hw_output, xo);
            blury.compute_at(hw_output, xo).store_at(hw_output, xo);          
            blurz.compute_at(hw_output, xo).store_at(hw_output, xo);

            blur_count_x.compute_at(hw_output, xo);
            blur_count_y.compute_at(hw_output, xo);
            blur_count_z.compute_at(hw_output, xo);
            
            input_copy.compute_at(hw_output, xo);
            hw_input.stream_to_accelerator();
        } else {
            // The CPU schedule.
           
        }


    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(BilateralGrid, bilateral)
