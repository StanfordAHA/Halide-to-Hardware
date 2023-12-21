#include "Halide.h"

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

const int inImgSize = 64;
//const int outImgSize = inImgSize - 2;
const int outImgSizeX = inImgSize - 4;
const int outImgSizeY = inImgSize - 4;

class ConvolutionKernel : public Halide::Generator<ConvolutionKernel> {
public:
    Input<Buffer<uint8_t>>  input{"input", 2};
    Output<Buffer<uint8_t>> output{"output", 2};
    GeneratorParam<uint8_t> schedule{"schedule", 1};    // default: 1

  //Input<int32_t> outImgSizeX{"outImgSizeX", 64, 8, 128};    // default: 64. bounded between 8 and 128
  //Input<int32_t> outImgSizeY{"outImgSizeY", 64, 8, 128};    // default: 64. bounded between 8 and 128

    void generate() {
        /* THE ALGORITHM */
      
        Var x("x"), y("y");

        Func kernel("kernel");
        //Func conv1("conv1"), conv2("conv2");
        RDom r(0, 3,
               0, 3);
        RDom r2(0, 3,
                0, 3);

        kernel(x,y) = 0;
        kernel(0,0) = 1;      kernel(0,1) = 2;      kernel(0,2) = 1;
        kernel(1,0) = 2;      kernel(1,1) = 4;      kernel(1,2) = 2;
        kernel(2,0) = 1;      kernel(2,1) = 2;      kernel(2,2) = 1;

        Func conv1("conv1"), conv1_shift("conv1_shift");
        Func conv2("conv2");

        Func hw_input("hw_input"), hw_input_copy;
        hw_input(x, y) = cast<uint16_t>(input(x, y));
        //hw_input(x, y) = hw_input_copy(x, y);

        if (schedule == 1 || schedule == 2) {
          conv1(x, y) = u16(0);
          conv1(x, y)  += u16(kernel(r.x, r.y)) * hw_input(x + r.x, y + r.y);
        } else {
          conv1(x, y)  =
            u16(kernel(0, 0)) * hw_input(x, y)
            + u16(kernel(1, 0)) * hw_input(x+1, y)
            + u16(kernel(2, 0)) * hw_input(x+2, y)
            + u16(kernel(0, 1)) * hw_input(x+0, y+1)
            + u16(kernel(1, 1)) * hw_input(x+1, y+1)
            + u16(kernel(2, 1)) * hw_input(x+2, y+1)
            + u16(kernel(0, 2)) * hw_input(x+0, y+2)
            + u16(kernel(1, 2)) * hw_input(x+1, y+2)
            + u16(kernel(2, 2)) * hw_input(x+2, y+2);
        }
        conv1_shift(x, y) = conv1(x,y) / 16;

        if (schedule == 1 || schedule == 2) {
          conv2(x, y) = u16(0);
          conv2(x, y)  += u16(kernel(r2.x, r2.y)) * conv1_shift(x + r2.x, y + r2.y);
        } else {
          conv2(x, y)  =
            u16(kernel(0, 0)) * conv1_shift(x, y)
            + u16(kernel(1, 0)) * conv1_shift(x+1, y)
            + u16(kernel(2, 0)) * conv1_shift(x+2, y)
            + u16(kernel(0, 1)) * conv1_shift(x+0, y+1)
            + u16(kernel(1, 1)) * conv1_shift(x+1, y+1)
            + u16(kernel(2, 1)) * conv1_shift(x+2, y+1)
            + u16(kernel(0, 2)) * conv1_shift(x+0, y+2)
            + u16(kernel(1, 2)) * conv1_shift(x+1, y+2)
            + u16(kernel(2, 2)) * conv1_shift(x+2, y+2);
        }

        Func hw_output("hw_output");
        hw_output(x, y) = conv2(x, y) / 16;
        //hw_output(x, y) = cast<uint8_t>(conv1(x, y));
        output(x, y) = cast<uint8_t>(hw_output(x,y));
        
        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR)) {

        } else if (get_target().has_feature(Target::Clockwork) || get_target().has_feature(Target::Pono) && (schedule == 2)) {
          Var xii, xio, yii, yio, xi, xo, yi, yo;
          output.bound(x,0,outImgSizeX).bound(y,0,outImgSizeY);
          
          hw_output.in().compute_root()
            .tile(x, y, xo, yo, xi, yi, 360, 360)
            .hw_accelerate(xi, xo)
            .store_in(MemoryType::GLB);
          hw_output
            .tile(x, y, xio, yio, xii, yii, 60, 60)
            .compute_at(hw_output.in(), xo);

          conv2.compute_at(hw_output, xio);
          conv2.update()
            .unroll(r2.x).unroll(r2.y);

          conv1.compute_at(hw_output, xio);
          conv1.update()
            .unroll(r.x).unroll(r.y);

          hw_input.in().in().compute_at(hw_output, xio);
          hw_input.in().compute_at(hw_output.in(), xo)
            .store_in(MemoryType::GLB);
          hw_input.compute_root().accelerator_input();

          kernel.compute_at(hw_output, xio);

        } else if (get_target().has_feature(Target::Clockwork) || get_target().has_feature(Target::Pono)) {
          Var xi,yi, xo,yo;

          output.bound(x, 0, outImgSizeX);
          output.bound(y, 0, outImgSizeY);

          hw_output.compute_root();
          hw_output
              .tile(x,y, xo,yo, xi,yi, outImgSizeX, outImgSizeY)
              .hw_accelerate(xi, xo);

          //hw_input.compute_root();
          //hw_input.store_at(hw_output, xo).compute_at(conv1, x);
          
          //kernel.compute_at(hw_output, yi);

          if (schedule == 1) {
            conv2.store_at(hw_output, xo).compute_at(hw_output, xo);
            conv2.update()
              //.reorder(r2.x, r2.y)
              .unroll(r2.x)
              .unroll(r2.y);
            
            conv1_shift.store_at(hw_output, xo).compute_at(hw_output, xo);
            conv1.store_at(hw_output, xo).compute_at(hw_output, xo);
            conv1.update()
              //.reorder(r.y, r.x)
              .unroll(r.x)
              .unroll(r.y);
          } else {
            //conv1.store_at(hw_output, xo).compute_at(hw_output, xo);
          }

          //kernel.compute_root();//.unroll(x).unroll(y);
          kernel.compute_at(hw_output, xo);

          //hw_input.compute_at(hw_output, xo);
          hw_input.stream_to_accelerator();
          //hw_input_copy.compute_root();
          
        } else {  // schedule to CPU
          //kernel.compute_root();
          //conv2.compute_root();
          //conv2.update()
            //.unroll(r.x, 3)
            //.unroll(r.y, 3);
        }
        
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(ConvolutionKernel, cascade)
