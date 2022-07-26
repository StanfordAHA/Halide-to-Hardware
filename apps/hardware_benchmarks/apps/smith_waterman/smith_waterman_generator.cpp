#include "Halide.h"

namespace {

using namespace Halide;

class SmithWatermanKernel : public Halide::Generator<SmithWatermanKernel> {
public:
    Input<Buffer<uint16_t>>  input_ref{"input_ref", 1};
    Input<Buffer<uint16_t>>  input_query{"input_query", 1};
    Output<Buffer<uint16_t>> output{"output", 2};

    void generate() {
        /* THE ALGORITHM */

        Var x("x"), y("y");
        int ref_len = 10;
        int query_len = 10;
        RDom r(0, ref_len, 0, query_len);

        Func hw_ref, hw_query;
        hw_ref(x) = input_ref(x+1);
        hw_query(x) = input_query(x+1);

        Func sw("sw");
        sw(x, y) = cast<uint16_t>(0);
        sw(r.x, r.y) = max(select(hw_ref(r.x-1) == hw_query(r.x-1),
                                  sw(r.x-1, r.y-1) +2,
                                  sw(r.x-1, r.y-1) -1),
                           sw(r.x, r.y-1) -1,
                           sw(r.x-1, r.y) -1,
                           0);

        Func hw_output("hw_output");
        hw_output(x, y) = cast<uint16_t>(sw(x, y));
        output(x, y) = hw_output(x,y);

        output.bound(x, 0, 10);
        output.bound(y, 0, 10);

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR)) {
        } else if (get_target().has_feature(Target::Clockwork)) {
          Var xi,yi, xo,yo;
          
          hw_ref.compute_root();
          hw_query.compute_root();
          hw_output.compute_root();
          
          hw_output.tile(x,y, xo,yo, xi,yi, 10, 10)
            .hw_accelerate(xi, xo);

          sw.compute_at(hw_output, xo);
          //remove the unroll to make compilation work, 
          //but it's good to see if we could support the unrolling schedule
          //sw.update().unroll(r.x);
          
          hw_ref.stream_to_accelerator();
          hw_query.stream_to_accelerator();
          
        } else {  // schedule to CPU
          output.compute_root();
        }
        
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(SmithWatermanKernel, smith_waterman)
