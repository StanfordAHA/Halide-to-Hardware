#include "Halide.h"

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

const int inImgSize = 64;
//const int outImgSize = inImgSize - 2;
const int ksize = 3;
const int num_kernels = 2;
const int outImgSizeX = inImgSize - num_kernels*(ksize-1);
const int outImgSizeY = inImgSize - num_kernels*(ksize-1);

class ConvolutionKernel : public Halide::Generator<ConvolutionKernel> {
public:
    Input<Buffer<uint8_t>>  input{"input", 2};
    Output<Buffer<uint8_t>> output{"output", 2};
    GeneratorParam<uint8_t> schedule{"schedule", 3};    // default: 3
    GeneratorParam<uint16_t> mywidth{"mywidth", 639};    // old default 368
    GeneratorParam<uint16_t> myheight{"myheight", 0};    // default: don't use
    GeneratorParam<uint16_t> myunroll{"myunroll", 1};
  
  //Input<int32_t> outImgSizeX{"outImgSizeX", 64, 8, 128};    // default: 64. bounded between 8 and 128
  //Input<int32_t> outImgSizeY{"outImgSizeY", 64, 8, 128};    // default: 64. bounded between 8 and 128

    void generate() {
        /* THE ALGORITHM */
      
        Var x("x"), y("y");

        Func kernel("kernel");
        //Func conv1("conv1"), conv2("conv2");
        RDom r(0, ksize,
               //0, 1);
               0, ksize);
        RDom r2(0, ksize,
                //0, 1);
                0, ksize);

        kernel(x,y) = 0;
        kernel(0,0) = 1;      kernel(0,1) = 2;      kernel(0,2) = 1;
        kernel(1,0) = 2;      kernel(1,1) = 4;      kernel(1,2) = 2;
        kernel(2,0) = 1;      kernel(2,1) = 2;      kernel(2,2) = 1;

        Func conv1("conv1"), conv1_shift("conv1_shift");
        Func conv2("conv2"), conv2_shift("conv2_shift");

        Func hw_input("hw_input"), hw_input_copy;
        hw_input(x, y) = cast<uint16_t>(input(x, y));
        //hw_input(x, y) = hw_input_copy(x, y);

        //Func conv0, conv0_shift;
        //conv0(x, y) = u16(0);
        //conv0(x, y)  += u16(kernel(r2.x, r2.y)) * hw_input(x + r2.x, y + r2.y);
        //conv0_shift(x,y) = conv0(x,y) / 16;


        if (schedule != 0) {
          conv1(x, y) = u16(0);
          //conv1(x, y)  += u16(kernel(r.x, r.y)) * conv0_shift(x + r.x, y + r.y);
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

        if (schedule != 0) {
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

        //Func conv2_shift, conv3;
        conv2_shift(x, y) = conv2(x,y) / 16;
        //conv3(x, y) = u16(0);
        //conv3(x, y) += u16(kernel(r2.x, r2.y)) * conv2_shift(x + r2.x, y + r2.y);
        

        Func hw_output("hw_output");
        //hw_output(x, y) = conv2(x, y) / 16;
        hw_output(x, y) = conv2_shift(x, y);
        //hw_output(x, y) = conv3(x, y) / 16;
        //hw_output(x, y) = cast<uint8_t>(conv1(x, y));
        output(x, y) = cast<uint8_t>(hw_output(x,y));
        
        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR)) {

        } else if (get_target().has_feature(Target::Clockwork) && (schedule == 2)) {
          Var xii, xio, yii, yio, xi, xo, yi, yo;
          output.bound(x,0,outImgSizeX).bound(y,0,outImgSizeY);
          
          hw_output.in().compute_root()
            .tile(x, y, xo, yo, xi, yi, 360, 360)
            .hw_accelerate(xi, xo);
          hw_output
            .tile(x, y, xio, yio, xii, yii, 60, 60)
            .compute_at(hw_output.in(), xo)
            .store_in(MemoryType::GLB);

          conv2.compute_at(hw_output, xio);
          conv2.update()
            .unroll(r2.x).unroll(r2.y);

          conv1_shift.compute_at(hw_output, xio);
          conv1.update()
            .unroll(r.x).unroll(r.y);//.unroll(x, 2);

          hw_input.in().in().compute_at(hw_output, xio);
          hw_input.in().compute_at(hw_output.in(), xo)
            .store_in(MemoryType::GLB);
          hw_input.compute_root().accelerator_input();

          kernel.compute_at(hw_output, xio);

        } else if (get_target().has_feature(Target::Clockwork) && (schedule == 3)) {
          const int unroll = myunroll; // 1, 2, 3 work
          const int tileWidth = mywidth;
          const int tileHeight = myheight==0 ? 99 : myheight; //799;

          const int numHostTilesX = 9;
          const int numHostTilesY = 5;
          const int outputWidth = numHostTilesX * tileWidth;
          const int outputHeight = numHostTilesY * tileHeight;
          output.bound(x, 0, outputWidth);
          output.bound(y, 0, outputHeight);

          Var xi("xi"),yi("yi"), xo("xo"),yo("yo");
          hw_output.in().compute_root();
          hw_output.in()
            .tile(x, y, xo, yo, xi, yi, tileWidth, tileHeight)
            .hw_accelerate(xi, xo);
          hw_output.in().unroll(xi, unroll, TailStrategy::RoundUp);


          Var xii("xii"), yii("yii"), xio("xio"), yio("yio");
          hw_output
            .tile(x, y, xio, yio, xii, yii, tileWidth, tileHeight);
          hw_output.compute_at(hw_output.in(), xo);
          hw_output.store_in(MemoryType::GLB);
          hw_output.unroll(xii, unroll, TailStrategy::RoundUp);

          conv2_shift.store_at(hw_output, xio).compute_at(hw_output, xio).unroll(x, unroll, TailStrategy::RoundUp);
          
          conv2.store_at(hw_output, xio).compute_at(hw_output, xio)
            .unroll(x, unroll, TailStrategy::RoundUp);
          conv2.update()
            //.reorder(r2.x, r2.y)
            .unroll(r2.x)
            .unroll(r2.y)
            .unroll(x, unroll, TailStrategy::RoundUp);
            
          conv1_shift.store_at(hw_output, xio).compute_at(hw_output, xio).unroll(x, unroll, TailStrategy::RoundUp);
          
          conv1.store_at(hw_output, xio).compute_at(hw_output, xio)
            .unroll(x, unroll, TailStrategy::RoundUp);
          conv1.update()
            //.reorder(r.y, r.x)
            .unroll(r.x)
            .unroll(r.y)
            .unroll(x, unroll, TailStrategy::RoundUp);

          hw_input.in().in().compute_at(hw_output, xio); // represents the mem tile
          hw_input.in().in().unroll(x, unroll, TailStrategy::RoundUp);

          hw_input.in().compute_at(hw_output.in(), xo);
          hw_input.in().store_in(MemoryType::GLB);
          hw_input.in().unroll(x, unroll, TailStrategy::RoundUp);
            
          hw_input.compute_root()
            .accelerator_input();

          kernel.compute_at(hw_output, xio);
          
        } else if (get_target().has_feature(Target::Clockwork)) {
          Var xi("xi"),yi("yi"), xo("xo"),yo("yo");

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
            
          } else if (schedule == 4) {
            conv2.store_at(hw_output, xo).compute_at(hw_output, yi);
            conv2.update()
              //.reorder(r2.x, r2.y)
              .unroll(r2.x)
              .unroll(r2.y);
            
            conv1_shift.store_at(hw_output, xo).compute_at(hw_output, yi);
            conv1.store_at(hw_output, xo).compute_at(hw_output, yi);
            conv1.update()
              //.reorder(r.y, r.x)
              .unroll(r.x)
              .unroll(r.y);            

          } else if (schedule == 11) {
            // create compute share schedule
            conv2.store_at(hw_output, xo).compute_at(hw_output, xo);
            //conv2.store_at(hw_output, xo).compute_at(hw_output, xi);
            conv2.update()
              .unroll(r2.x)
              .unroll(r2.y);
            
            conv1_shift.store_at(hw_output, xo).compute_at(hw_output, xo);
            //conv1_shift.store_at(hw_output, xo).compute_at(hw_output, xi);
            conv1.store_at(hw_output, xo).compute_at(hw_output, xo);
            //conv1.store_at(hw_output, xo).compute_at(hw_output, xi);
            conv1.update()
              .unroll(r.x)
              .unroll(r.y);

            conv2.update(0).compute_share_root(y);
            conv1.update(0).compute_share(conv2.update(0));
            //conv2.compute_share_root(y);
            //conv1.compute_share(conv2);

          } else if (schedule == 12) {
            // create compute share schedule with yo loop fusion
            Var cl = yi;
            conv2.store_at(hw_output, xo).compute_at(hw_output, cl);
            //conv2.store_at(hw_output, cl).compute_at(hw_output, xi);
            conv2.update()
              .unroll(r2.x)
              .unroll(r2.y);
            
            conv1_shift.store_at(hw_output, xo).compute_at(hw_output, cl);
            //conv1_shift.store_at(hw_output, cl).compute_at(hw_output, xi);
            conv1.store_at(hw_output, xo).compute_at(hw_output, cl);
            //conv1.store_at(hw_output, cl).compute_at(hw_output, xi);
            conv1.update()
              .unroll(r.x)
              .unroll(r.y);

            //conv2.update(0).compute_share_root(y);
            //conv1.update(0).compute_share(conv2.update(0));
            //conv2.compute_share_root(y);
            //conv1.compute_share(conv2);

          } else if (schedule == 13) {
            // create compute share schedule with yo loop fusion
            hw_output.coarse_grain_loop(yi);

            Var yz;
            Var cl = xo;
            conv2.store_at(hw_output, cl).compute_at(hw_output, cl);
            //conv2.store_at(hw_output, cl).compute_at(hw_output, xi);
            conv2.update()
              .unroll(r2.x)
              .unroll(r2.y)
              .coarse_grain_loop(y);
            conv2.coarse_grain_loop(y);
            
            conv1_shift.store_at(hw_output, cl).compute_at(hw_output, cl);
            conv1_shift.coarse_grain_loop(y);
            //conv1_shift.store_at(hw_output, cl).compute_at(hw_output, xi);
            conv1.store_at(hw_output, cl).compute_at(hw_output, cl);
            conv1.coarse_grain_loop(y);
            //conv1.store_at(hw_output, cl).compute_at(hw_output, xi);
            conv1.update()
              .unroll(r.x)
              .unroll(r.y)
              .coarse_grain_loop(y);

            hw_input.in().coarse_grain_loop(y);
            
          } else {
            //conv1.store_at(hw_output, xo).compute_at(hw_output, xo);
          }

          //kernel.compute_root();//.unroll(x).unroll(y);
          kernel.compute_at(hw_output, xo);

          //hw_input.compute_at(hw_output, xo);
          hw_input.stream_to_accelerator();
          //hw_input_copy.compute_root();
          
        } else {  // schedule to CPU

          // no scheduling: 735.595ms
          // all compute root: 249.476ms
          // conv1_shift compute root only: 63.9487ms
          // conv1_shift compute at xo, 512x32 tiles: 33.2511ms
          // ..., unroll conv{1,2} 29.9316ms
          // ... parallel output yo: 2.61347ms

          const int vec_size = 16;
          
          Var xi("xi"), yi("yi"), xo("xo"), yo("yo");

          if (schedule == 50) {
            //Manually-tuned time: 3317.69ms
            output.compute_inline();
            hw_output.compute_inline();
            conv2_shift.compute_inline();
            conv2.compute_inline();
            conv1_shift.compute_inline();
            conv1.compute_inline();
            hw_input.compute_inline();
            kernel.compute_inline();

          } else if (schedule == 51) {
            //Manually-tuned time: 628.372ms
            output.compute_root();
            hw_output.compute_root();
            conv2_shift.compute_root();
            conv2.compute_root();
            conv1_shift.compute_root();
            conv1.compute_root();
            hw_input.compute_root();
            kernel.compute_root();

          } else if (schedule == 52) {
            //Manually-tuned time: 759.028ms
            conv1_shift.compute_root();

          } else if (schedule == 53) {
            //Manually-tuned time: 3334.8ms
            output
              .compute_root()
              .tile(x, y, xo, yo, xi, yi, 512, 32);
            conv1_shift.compute_root();

          } else if (schedule == 59) {
            //Manually-tuned time: 3341.87ms
            
          } else if (schedule == 58) {
            //Manually-tuned time: 3643.11ms
            output
              .tile(x, y, xo, yo, xi, yi, 512, 32);
            
          } else if (schedule == 54) {
            //Manually-tuned time: 394.655ms
            output
              .compute_root()
              .tile(x, y, xo, yo, xi, yi, 512, 32);
            
            conv1_shift.compute_at(output, xo);
            kernel.compute_at(output, xo);

          } else if (schedule == 64) {
            //Manually-tuned time: 394.655ms
            output
              .compute_root()
              .tile(x, y, xo, yo, xi, yi, 512, 32);
            
            conv1_shift.store_at(output, xo).compute_at(output, yi);
            kernel.compute_at(output, xo);

            
          } else if (schedule == 55) {
            //Manually-tuned time: 124.455ms
            output
              .compute_root()
              .tile(x, y, xo, yo, xi, yi, 512, 32);

            conv2.update().unroll(r2.x).unroll(r2.y);
            
            conv1.update().unroll(r.x).unroll(r.y);
            
            conv1_shift.compute_at(output, xo);
            kernel.compute_at(output, xo);

          } else if (schedule == 65) {
            //Manually-tuned time: 124.455ms
            output
              .compute_root()
              .tile(x, y, xo, yo, xi, yi, 512, 32);

            conv2.update().unroll(r2.x).unroll(r2.y);
            
            conv1.update().unroll(r.x).unroll(r.y);
            
            conv1_shift.store_at(output, xo).compute_at(output, yi);
            kernel.compute_at(output, xo);
            
          } else if (schedule == 56) {
            //Manually-tuned time: 31.7394ms
            const int vec = 16;
            output
              .compute_root()
              .tile(x, y, xo, yo, xi, yi, 512, 32)
              .vectorize(xi, vec);

            conv2.update().unroll(r2.x).unroll(r2.y);
            
            conv1.update().unroll(r.x).unroll(r.y);
            
            conv1_shift
              .compute_at(output, xo)
              .vectorize(x, vec);
            kernel.compute_at(output, xo);
            //.vectorize(x, vec);

          } else if (schedule == 57) {
            //Manually-tuned time: 2.96118ms
            const int vec = 16;
            output
              .store_root().compute_root()
              .tile(x, y, xo, yo, xi, yi, 512, 32)
              .vectorize(xi, vec)
              .parallel(yo);

            conv2.update()
              .unroll(r2.x).unroll(r2.y);
            
            conv1.update()
              .unroll(r.x).unroll(r.y);
            
            conv1_shift
              .store_at(output, xo).compute_at(output, xo)
              .vectorize(x, vec);
            kernel
              .store_at(output, xo).compute_at(output, xo);
              //.vectorize(x, vec);

          } else if (schedule == 67) {
            //Manually-tuned time: 2.60584ms
            const int vec = 16;
            output
              .store_root().compute_root()
              .tile(x, y, xo, yo, xi, yi, 512, 32)
              .vectorize(xi, vec)
              .parallel(yo);

            conv2.update()
              .unroll(r2.x).unroll(r2.y);
            
            conv1.update()
              .unroll(r.x).unroll(r.y);
            
            conv1_shift
              .store_at(output, xo).compute_at(output, yi)
              .vectorize(x, vec);
            kernel
              .store_at(output, xo).compute_at(output, xo);
              //.vectorize(x, vec);

            
            
            
          } else {
          
            output
              .compute_root()
              .tile(x, y, xo, yo, xi, yi, 512, 32)
              .parallel(yo)
              .vectorize(xi, vec_size);

            //conv2.compute_at(output, xo);
            conv2.update().unroll(r2.x).unroll(r2.y);
          
            //hw_output.compute_root();
            //conv2_shift.compute_root();
            //conv2.compute_root();
            conv1_shift
              //.store_at(output, xo)
              .compute_at(output, xo)
              .vectorize(x, vec_size);
          
            //conv1.compute_at(output, xo);
            conv1.update().unroll(r.x).unroll(r.y);
            //conv1.compute_root();

            kernel.unroll(x).unroll(y);
            kernel.in(conv2).compute_at(conv2, y)
              .unroll(x).unroll(y);//Var::outermost);
            //kernel.in(conv1).compute_at(conv1, Var::outermost);
            //kernel
              //.unroll(x).unroll(y)
              //.compute_at(output, xo)
              //.vectorize(x, vec_size);

            //conv2.update()
            //.unroll(r.x, 3)
            //.unroll(r.y, 3);
          }
        }
        
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(ConvolutionKernel, cascade)
