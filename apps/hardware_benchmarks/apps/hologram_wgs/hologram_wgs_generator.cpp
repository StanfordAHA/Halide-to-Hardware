#include "Halide.h"
#include "complex.h"
#include <math.h>

namespace {

class HologramWGS : public Halide::Generator<HologramWGS> {
public:
    Input<Buffer<float>> in_x{"in_x", 1};
    Input<Buffer<float>> in_y{"in_y", 1};
    Input<Buffer<float>> in_z{"in_z", 1};
    Output<Buffer<float>> phases{"phases", 2};

    void generate() {
        // parameters
        float f = 20.0f * 1000.0f;
        float d = 15.0f;
        float lam = 0.488f;
        int res = 512;
        int iters = 2;
        //Expr num_traps = in_x.width();
        int num_traps = 100;

        Func in_x_copy("in_x_copy"), in_y_copy("in_y_copy"), in_z_copy("in_z_copy");
        Func hw_in_x("hw_in_x"), hw_in_y("hw_in_y"), hw_in_z("hw_in_z");
        Func slm_xcoord("slm_xcoord");
        Func slm_ycoord("slm_ycoord");
        Func weights[iters + 1];
        Func pists[iters + 1];
        Func slm_p_phase("slm_p_phase");
        Func slm_total_phase[iters];
        Func phases_copy("phases_copy");
        Func hw_output("hw_output");
        Var x("x"), y("y"), z("z");

        RDom slm_dim = RDom(0, res, 0, res);
        RDom traps_dim = RDom(0, num_traps);

        Var xi, xo, yi, yo;

        // The algorithm
        
        in_x_copy(x) = cast<float>(in_x(x));
        in_y_copy(x) = cast<float>(in_y(x));
        in_z_copy(x) = cast<float>(in_z(x));

        hw_in_x(x) = in_x_copy(x);
        hw_in_y(x) = in_y_copy(x);
        hw_in_z(x) = in_z_copy(x);

        slm_xcoord(x, y) = (x * (res + 1.0f) / res - res / 2.0f) * d;
        slm_ycoord(x, y) = (y * (res + 1.0f) / res - res / 2.0f) * d;
        weights[0](x) = 1.0f / (num_traps);
        pists[0](x) = 2.0f*float(M_PI)*float(rand())/RAND_MAX;

        slm_p_phase(x, y, z) = float(M_PI) * hw_in_z(x) / (lam * f * f) * (slm_xcoord(y, z) * slm_xcoord(y, z) + slm_ycoord(y, z) * slm_ycoord(y, z)) + 2 * float(M_PI) / (lam * f) * (slm_xcoord(y, z) * hw_in_x(x) + slm_ycoord(y, z) * hw_in_y(x));

        for (int i = 0; i < iters; i++) {
            // local to each iteration
            ComplexFunc slm_total_field("slm_total_field");
            Func ints("ints");
            Func avg_ints("avg_ints");
            ComplexFunc spot_fields("spot_fields");
            Func weights_new("weights_new");
            Func sum_weights("sum_weights");

            slm_total_field(x, y) = ComplexExpr(0.0f, 0.0f);
            slm_total_field(x, y) += (weights[i](traps_dim.x) / res) *
                                     expj(slm_p_phase(traps_dim.x, x, y) +
                                          pists[i](traps_dim.x));

            slm_total_phase[i](x, y) = arg(slm_total_field(x, y));

            spot_fields(x) = ComplexExpr(0.0f, 0.0f);
            spot_fields(x) += (1.0f / (res*res)) * expj(slm_total_phase[i](slm_dim.x, slm_dim.y) -
                                                  slm_p_phase(x, slm_dim.x, slm_dim.y));

            pists[i + 1](x) = arg(spot_fields(x));

            ints(x) = abs2(spot_fields(x));
            avg_ints() = sum(sqrt(ints(traps_dim.x))) / num_traps;

            weights_new(x) = weights[i](x) * (avg_ints()) / sqrt(ints(x));
            sum_weights() = sum(weights_new(traps_dim.x));
            weights[i + 1](x) = weights_new(x) / sum_weights();

            if (auto_schedule) {
                // don't do anything
            } else if (get_target().has_feature(Target::CoreIR)) {
            } else if (get_target().has_feature(Target::Clockwork) || get_target().has_feature(Target::Pono)) {
                //ints.compute_root();
                //avg_ints.compute_root();
                //weights_new.compute_root();
                //sum_weights.compute_root();
                //pists[i+1].compute_root();
                //weights[i+1].compute_root();

                spot_fields.update()
                    .unroll(slm_dim.x, 2)
                    .unroll(slm_dim.y, 2);
                //slm_total_phase[i].compute_at(phases, Var::outermost());
                //slm_total_phase[i].compute_root();
                slm_total_field.update()
                    .unroll(traps_dim.x, 2);

                //slm_p_phase.in(slm_total_field).compute_at(slm_total_field, Var::outermost());
            } else {
                // CPU schedule
                // Don't try to parallelize these things. They're small.
                ints.compute_root().vectorize(x, 8);
                avg_ints.compute_root();
                weights_new.compute_root().vectorize(x, 8);
                sum_weights.compute_root();
                pists[i + 1].compute_root().vectorize(x, 8);
                weights[i + 1].compute_root().vectorize(x, 8);

                // These two Funcs are where the real compute is. Do one vector of the output per thread.
                spot_fields.compute_root()
                    .vectorize(x, 8)
                    .update()
                    .vectorize(x, 8)
                    .parallel(x);

                slm_total_phase[i]
                    .compute_root()
                    .vectorize(x, 8)
                    .parallel(y);

                slm_total_field.compute_at(slm_total_phase[i], y)
                    .vectorize(x, 8)
                    .update()
                    .reorder(x, traps_dim.x, y)
                    .vectorize(x, 8);
            }
        }

        phases_copy = slm_total_phase[iters - 1];

        hw_output(x,y) = phases_copy(x,y);
        phases(x,y) = hw_output(x,y);

        if (auto_schedule) {
        } else if (get_target().has_feature(Target::CoreIR)) {
        } else if (get_target().has_feature(Target::Clockwork) || get_target().has_feature(Target::Pono)) {
            //hw_in_x.bound(x, 0, num_traps);
            //hw_in_y.bound(x, 0, num_traps);
            //hw_in_z.bound(x, 0, num_traps);
            phases.bound(x, 0, res);
            phases.bound(y, 0, res);

            hw_output.compute_root();
            hw_output.tile(x, y, xo, yo, xi, yi, res, res)
                .hw_accelerate(xi, xo);

            //slm_xcoord.compute_root();
            //slm_ycoord.compute_root();
            //slm_p_phase.compute_root();
            //weights[0].compute_root();
            //pists[0].compute_root();
            
            //slm_xcoord.compute_at(phases, xo);
            //slm_ycoord.compute_at(phases, xo);
            //slm_p_phase.compute_at(phases, xo);
            //weights[0].compute_at(phases, xo);
            //pists[0].compute_at(phases, xo);

            //hw_in_x.compute_at(slm_p_phase, Var::outermost());
            //hw_in_y.compute_at(slm_p_phase, Var::outermost());
            //hw_in_z.compute_at(slm_p_phase, Var::outermost());
            //hw_in_x.compute_root();
            //hw_in_y.compute_root();
            //hw_in_z.compute_root();

            in_x_copy.accelerator_input();
            in_y_copy.accelerator_input();
            in_z_copy.accelerator_input();

            in_x_copy.compute_root();
            in_y_copy.compute_root();
            in_z_copy.compute_root();
        } else {
            // CPU schedule
            slm_p_phase.compute_root()
                .parallel(z)
                .reorder_storage(y, x, z)
                .vectorize(y, 8);
        }
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(HologramWGS, hologram_wgs)
