#include "Halide.h"

namespace {

using namespace std;

using namespace Halide;

class PEMemFlushTest : public Halide::Generator<PEMemFlushTest> {
public:
    Input<Buffer<uint16_t>>  input{"input", 2};
    Output<Buffer<uint16_t>> output{"output", 2};

    GeneratorParam<int32_t> myunroll{"myunroll", 1};
    GeneratorParam<int32_t> myunroll_E64{"myunroll_E64", 8};
    GeneratorParam<int32_t> myunroll_E64_MB{"myunroll_E64_MB", 16};

    GeneratorParam<int32_t> input_width{"input_width", 128};
    GeneratorParam<int32_t> input_height{"input_height", 128};

    GeneratorParam<int32_t> mem_column_idx{"mem_column_idx", 15};
    GeneratorParam<int32_t> column_start_y{"column_start_y", 1};
    GeneratorParam<int32_t> mult_const{"mult_const", 2};

    void generate() {
        /* THE ALGORITHM */

        Var x("x"), y("y");

        Func input_copy, hw_input, hw_output;
        Func mult("mult");
        //input_copy(x, y) = cast<uint16_t>(input(x, y));
        //hw_input(x, y) = input_copy(x, y);
        hw_input(x, y) = cast<uint16_t>(input(x, y));

        mult(x, y) = hw_input(x,y) * cast<uint16_t>(mult_const);
        hw_output(x, y) = mult(x, y);
        output(x, y) = cast<uint16_t>(hw_output(x, y));

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR) ||
            get_target().has_feature(Target::HLS)) {
          Var xi,yi, xo,yo;

          //hw_input.compute_root();
          hw_output.compute_root();
          hw_output.tile(x,y, xo,yo, xi,yi, input_width, input_height)
            .hw_accelerate(xi, xo);
          //hw_output.tile(x,y, xo,yo, xi,yi, 64, 64-2)
            //.hw_accelerate(xi, xo);
          output.bound(x, 0, input_width);
          output.bound(y, 0, input_height);

          hw_input.store_at(hw_output, xo).compute_at(hw_output, xi);

          hw_input.stream_to_accelerator();

          cout << "Loop nest" << endl;
          hw_output.print_loop_nest();

        } else if (get_target().has_feature(Target::Clockwork)) {
          Var xi,yi, xo,yo;

          const char* e64_mode_env = getenv("E64_MODE_ON");
          const char* e64_multi_bank_mode_env = getenv("E64_MULTI_BANK_MODE_ON");
          int unroll = myunroll;
          if (e64_multi_bank_mode_env && std::stoi(e64_multi_bank_mode_env) == 1) {
            unroll = myunroll_E64_MB;
          } else if (e64_mode_env && std::stoi(e64_mode_env) == 1) {
            unroll = myunroll_E64;
          }
          output.bound(x, 0, input_width);
          output.bound(y, 0, input_height);

          //hw_input.compute_root();
          hw_output.compute_root();
          hw_output
              .tile(x,y, xo,yo, xi,yi, input_width, input_height)
              .hw_accelerate(xi, xo);
          hw_output.unroll(xi, unroll);

          mult.compute_at(hw_output, xo)
            .unroll(x, unroll);

          //hw_input.store_at(hw_output, xo).compute_at(hw_output, xo);
          hw_input.stream_to_accelerator();
          hw_input.in().unroll(x, unroll);
          //input_copy.compute_root();

        } else {
          mult.compute_root();
        }
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(PEMemFlushTest, pe_mem_flush_test)
