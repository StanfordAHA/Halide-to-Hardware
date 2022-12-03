#include "Halide.h"
#include <algorithm>
#include <stdio.h>
#include <math.h>
#include "complex.h"

namespace {

using namespace std;
using namespace Halide;
using namespace Halide::ConciseCasts;

class FFT16 : public Halide::Generator<FFT16> {
public:
  Input<Buffer<uint16_t>> input{"input", 2};
  Output<Buffer<uint16_t>> output{"output", 2};

  void generate() {
		Var x, y;
		Func non_rev, rev, d9, d8, d7, d6, d5, d4, d3, d2, d1, d0;
		ComplexFunc stages;

    Func hw_input;
    hw_input(x, y) = bf16(input(x, y));
		
		// initialize
		stages(x, y) = ComplexExpr(bf16(0.0f), bf16(0.0f));
		//////////////////////////////////////////////
		
		// bit reverse
		non_rev(x) = x;
				 
		d3(x) = (non_rev(x) >> 3);
		d2(x) = (non_rev(x) >> 2) & 1;
		d1(x) = (non_rev(x) >> 1) & 1;
		d0(x) = non_rev(x) & 1;
		
		rev(x) = (d0(x) << 3)
      + (d1(x) << 2)
      + (d2(x) << 1)
      + d3(x);
		
		stages(x, 0) = ComplexExpr(hw_input(rev(x), 0), hw_input(rev(x), 1));
		//////////////////////////////////////////
		
		
		
		// twiddling factor
		ComplexFunc twi;
		RDom r = RDom(0, 8);
		
		float PI = atan(1)*4;
		twi(x) = ComplexExpr(bf16(0.0f), bf16(0.0f));
		twi(r.x) = expj_bf16(-PI * r.x / 8);
		///////////////////////////////////////////
		
		
		// stages
		for (int s = 1; s <= 4; s++)
      {
        RDom t = RDom(0, 16);
        int tmp = 1 << s;
			
        stages(t.x, s - 1) = stages(t.x, s - 1) * twi(((t.x % tmp) / (tmp / 2)) * (t.x % (tmp / 2)) * (16 / tmp));
			
        stages(t.x, s) = (1 - 2 * (t.x / ((t.x / tmp) * tmp + tmp / 2))) *
          stages(t.x, s - 1) +
          stages((t.x + tmp / 2) % tmp + (t.x / tmp) * tmp, s - 1);
      }

    Func hw_output;
    hw_output(x, y) = select(y==0, stages(x, 4).x,
                             /*y==1*/ stages(x, 4).y);

    output(x, y) = u16(hw_output(x, y));
    
		////////////////////////////////////////////////////////////////////////
		
		if (get_target().has_feature(Target::Clockwork)) {			  
      // Var xi, yi, xo, yo;
			  
      output.bound(x, 0, 16);
      output.bound(y, 0, 2);

      hw_output.compute_root();

      Var xi, yi, xo, yo;
      hw_output
        .tile(x, y, xo, yo, xi, yi, 16, 2)
        .hw_accelerate(xi, xo);
      
      //stages.compute_at(hw_output, xo);
      //twi.compute_at(hw_output, xo);
			  
      //stages.stream_to_accelerator();
      hw_input.stream_to_accelerator();
      //twi.stream_to_accelerator();
			  
    } else {
      stages.compute_root();
      twi.compute_root();
    }
  }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(FFT16, fft16)

// namespace {
// 
// using namespace std;
// using namespace Halide;
// using namespace Halide::ConciseCasts;
// 
// class FFT1024 : public Halide::Generator<FFT1024> {
// public:
//   Input<Buffer<float>> input{"in1", 2};
//   Output<Buffer<float>> output{"output", 2};
// 
//   void generate() {
// 		Var x, y, z;
// 		
// 		
// 		// bit reverse
// 		Func non_rev, rev, d9, d8, d7, d6, d5, d4, d3, d2, d1, d0;
// 		
// 		non_rev(x) = x;
// 				 
// 		d9(x) = (non_rev(x) >> 9);
// 		d8(x) = (non_rev(x) >> 8) & 1;
// 		d7(x) = (non_rev(x) >> 7) & 1;
// 		d6(x) = (non_rev(x) >> 6) & 1;
// 		d5(x) = (non_rev(x) >> 5) & 1;
// 		d4(x) = (non_rev(x) >> 4) & 1;
// 		d3(x) = (non_rev(x) >> 3) & 1;
// 		d2(x) = (non_rev(x) >> 2) & 1;
// 		d1(x) = (non_rev(x) >> 1) & 1;
// 		d0(x) = non_rev(x) & 1;
// 		
// 		rev(x) = (d0(x) << 9)
//       + (d1(x) << 8)
//       + (d2(x) << 7)
//       + (d3(x) << 6)
//       + (d4(x) << 5)
//       + (d5(x) << 4)
//       + (d6(x) << 3)
//       + (d7(x) << 2)
//       + (d8(x) << 1)
//       + d9(x);
// 		//////////////////////////////////////////
// 		
// 
// 
// 		// twiddling factor
// 		ComplexFunc twi;
// 		
// 		float PI = atan(1)*4;
// 		twi(x) = expj(-2 * PI * x / 1024);
// 		///////////////////////////////////////////
// 		
// 
// 		
// 		// stages
// 		Func hw_input, hw_output;
// 		ComplexFunc stages("stages");
// 		RDom t = RDom(0, 1024);
//     int num_stages = 10;
//     RDom r = RDom(0, 1024, 0, num_stages);
// 		
// 		stages(x, y) = ComplexExpr(0.0f, 0.0f);
// 		
// 		hw_input(x, y) = input(x, y);
// 		stages(x, 0) = ComplexExpr(hw_input(rev(x), 0), hw_input(rev(x), 1));
// 		
// 		for (int s = 1; s <= 10; s++) {
//         auto tmp = 1 << s;
// 			
//         stages(t.x, s - 1) = stages(t.x, s - 1) * twi(((t.x % tmp) / (tmp / 2)) * (t.x % (tmp / 2)) * (1024 / tmp));
// 			  
//         stages(t.x, s) = (1 - 2 * (t.x / ((t.x / tmp) * tmp + tmp / 2))) *
//           stages(t.x, s - 1) +
//           stages((t.x + tmp / 2) % tmp + (t.x / tmp) * tmp, s - 1);
// 
//         //auto tmp = 1 << r.y;
//         //stages(r.x, r.y - 1) = stages(r.x, r.y - 1) * twi(((r.x % tmp) / (tmp / 2)) * (r.x % (tmp / 2)) * (1024 / tmp));
//         //
//         //stages(r.x, r) = (1 - 2 * (r.y / ((r.y / tmp) * tmp + tmp / 2))) *
//         //  stages(r.x, r.y - 1) +
//         //  stages((r.x + tmp / 2) % tmp + (r.x / tmp) * tmp, r.y - 1);
//       }
// 		
// 		hw_output(x, y) = select(y==0, stages(x, 10).x, stages(x, 10).y);
// 		output(x, y) = hw_output(x, y);
// 		////////////////////////////////////////////////////////////////////////
// 		
// 		//stages.bound(z, 0, 1024);
// 		
// 		if (get_target().has_feature(Target::Clockwork)) {			  
// 			  Var xi, yi, xo, yo;
// 					  
// 			  output.bound(x, 0, 1024);
// 			  output.bound(y, 0, 2);
// 
// 			  hw_output.compute_root();
// 
//         //twi
//         //  .compute_at(hw_output, xo)
//         //  //.store_in(MemoryType::Stack)
//         //  .unroll(x, 1024);
//         
// 			  hw_output
//           .tile(x, y, xo, yo, xi, yi, 1024, 2)
//           .hw_accelerate(xi, xo);
// 			  
// 			  hw_input.stream_to_accelerator();
// 			  
//       } else {
//         stages.compute_root();
//         twi.compute_root();
//       }
//   }
// };
// 
// }  // namespace
// 
// HALIDE_REGISTER_GENERATOR(FFT1024, fft1024)
// 