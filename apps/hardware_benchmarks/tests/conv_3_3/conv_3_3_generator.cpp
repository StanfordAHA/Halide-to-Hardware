#include "Halide.h"

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

class ConvolutionKernel : public Halide::Generator<ConvolutionKernel> {
public:
    Input<Buffer<uint8_t>>  input{"input", 2};
    Output<Buffer<uint8_t>> output{"output", 2};

  GeneratorParam<int>     in_img{"in_img", 64};        // image side; tilesize = in_img - ksize + 1
  GeneratorParam<uint8_t> schedule{"schedule", 0};     // 0=single-level, 1=GLB+MEM, 2=host+GLB+MEM
  GeneratorParam<int>     glb_x{"glb_x", 0};           // 0 = auto = tilesize
  GeneratorParam<int>     glb_y{"glb_y", 0};
  GeneratorParam<int>     mem_x{"mem_x", 0};           // 0 = auto = glb_x
  GeneratorParam<int>     mem_y{"mem_y", 0};
  GeneratorParam<int>     sub_mem_x{"sub_mem_x", 0};   // schedule=4 inner tile; 0 = auto = mem_x/2
  GeneratorParam<int>     sub_mem_y{"sub_mem_y", 0};
  GeneratorParam<int>     unroll{"unroll", 3};         // RDom unroll factor; default ksize
  int ksize = 3;

    void generate() {
        /* THE ALGORITHM */

        Var x("x"), y("y");

        Func kernel("kernel");
        Func conv("conv");
        RDom r(0, ksize,               0, ksize);

        kernel(x,y) = 0;
//        kernel(0,0) = 17;      kernel(1,0) = 4;        kernel(2,0) = 6;
//        kernel(0,1) = 7;       kernel(1,1) = 19;       kernel(2,1) = 4;
//        kernel(0,2) = 5;       kernel(1,2) = 21;       kernel(2,2) = 15;
//        kernel(0,0) = 1;      kernel(1,0) = 1;       kernel(2,0) = 1;
//        kernel(0,1) = 1;      kernel(1,1) = 1;       kernel(2,1) = 1;
//        kernel(0,2) = 1;      kernel(1,2) = 1;       kernel(2,2) = 1;
        kernel(0,0) = 11;      kernel(0,1) = 12;      kernel(0,2) = 13;
        kernel(1,0) = 0;       kernel(1,1) = 0;       kernel(1,2) = 16;
        kernel(2,0) = 17;      kernel(2,1) = 18;      kernel(2,2) = 19;
        
        conv(x, y) = u16(0);

        Func hw_input("hw_input");
        Func hw_output("hw_output");
        Func output_glb("output_glb");

        // Named intermediate output_glb (mirroring apps/conv2D_fp/conv2D_fp_generator.cpp) sits
        // between conv (compute) and hw_output (boundary). Gives the GLB write stage its own
        // iteration domain so it can be tiled at MEM sub-tile grain. Inlined by default when
        // not explicitly scheduled — schedules 0 and 2 see no change.
        //
        // A fully symmetric input/output rewrite (input_glb, input_cgra + hw_output tiled at MEM)
        // was attempted 2026-07-13 and hit clockwork-side constraints: "No viable banking
        // strategy for input_cgra_stencil" and "Cannot select flush From output_glb_..._ubuf".
        // conv2D_fp uses this pattern successfully but only because the whole app is authored to
        // fit clockwork's ubuffer-topology expectations. Grafting the pattern onto conv_3_3
        // breaks the assumptions. See /aha/clockwork/CLAUDE.md.
        hw_input(x, y) = u16(input(x, y));

        conv(x, y) += u16(kernel(r.x, r.y)) * hw_input(x + r.x, y + r.y);

        output_glb(x, y) = conv(x, y);
        hw_output(x, y) = output_glb(x, y);
        output(x, y) = cast<uint8_t>(hw_output(x,y));

        /* THE SCHEDULE */
        int tilesize_x = (int)in_img - ksize + 1;
        int tilesize_y = (int)in_img - ksize + 1;
        int eff_glb_x  = (int)glb_x ? (int)glb_x : tilesize_x;
        int eff_glb_y  = (int)glb_y ? (int)glb_y : tilesize_y;
        int eff_mem_x  = (int)mem_x ? (int)mem_x : eff_glb_x;
        int eff_mem_y  = (int)mem_y ? (int)mem_y : eff_glb_y;
        int eff_sub_mem_x = (int)sub_mem_x ? (int)sub_mem_x : (eff_mem_x / 2);
        int eff_sub_mem_y = (int)sub_mem_y ? (int)sub_mem_y : (eff_mem_y / 2);

        if (get_target().has_feature(Target::CoreIR) ||
            get_target().has_feature(Target::HLS)) {
          Var xi,yi, xo,yo;
          output.bound(x, 0, tilesize_x);
          output.bound(y, 0, tilesize_y);
//          Var x_host,y_host, x_gb,y_gb, x_cgra,y_cgra;
//          // Produce loop levels: host, global buffer, cgra
//          output.tile(x, y, x_host,y_host, xi,yi, 256,256);
//          output.tile(xi, yi, x_gb,y_gb, x_cgra,y_cgra, 64-2,64-2);

//          hw_input.store_root().compute_root();
//          hw_input.in().store_at(output, x_host).compute_at(output,x_gb);
//          hw_input.in().in().store_at(output, x_gb).compute_at(output,x_cgra);
          hw_output.compute_root();
          hw_output.tile(x,y, xo,yo, xi,yi, tilesize_x, tilesize_y)
            .hw_accelerate(xi, xo);
          conv.update()
            .unroll(r.y, ksize)
            .unroll(r.x, ksize);
          conv.linebuffer();
          //kernel.store_at(hw_output, yi).compute_at(hw_output, yi);
          hw_input.stream_to_accelerator();

        } else if (get_target().has_feature(Target::Clockwork)) {
          output.bound(x, 0, tilesize_x);
          output.bound(y, 0, tilesize_y);

          if ((uint8_t)schedule == 0) {
            // Single-level: whole image == one hw tile (original behavior, default).
            Var xi, yi, xo, yo;
            hw_output.compute_root();
            hw_output
                .tile(x, y, xo, yo, xi, yi, tilesize_x, tilesize_y)
                .hw_accelerate(xi, xo);
            kernel.compute_at(hw_output, xo);
            conv.compute_at(hw_output, xo);
            conv.update()
              .unroll(r.x, (int)unroll)
              .unroll(r.y, (int)unroll);
            hw_input.stream_to_accelerator();

          } else if ((uint8_t)schedule == 1) {
            // 2-level: GLB outer (eff_glb_x x eff_glb_y) + MEM/CGRA inner (eff_mem_x x eff_mem_y).
            // Pattern from apps/cascade/cascade_generator.cpp:87-112.
            Var xi, yi, xo, yo, xii, yii, xio, yio;
            hw_output.in().compute_root()
                .tile(x, y, xo, yo, xi, yi, eff_glb_x, eff_glb_y)
                .hw_accelerate(xi, xo)
                .store_in(MemoryType::GLB);
            hw_output
                .tile(x, y, xio, yio, xii, yii, eff_mem_x, eff_mem_y)
                .compute_at(hw_output.in(), xo);
            conv.compute_at(hw_output, xio);
            conv.update()
              .unroll(r.x, (int)unroll)
              .unroll(r.y, (int)unroll);
            hw_input.in().in().compute_at(hw_output, xio);
            hw_input.in().compute_at(hw_output.in(), xo).store_in(MemoryType::GLB);
            hw_input.compute_root().accelerator_input();
            kernel.compute_at(hw_output, xio);

          } else if ((uint8_t)schedule == 3) {
            // Path B: tile the wrapper at MEM grain so the wrapper's outer
            // iteration IS the sub-tile axis; hw_output computes each sub-tile
            // in scope. Goal: enable overlapped sub-tile execution (target
            // ~4095 cycles for split-mem, vs. 14535 with schedule==1's
            // whole-tile wrapper). See /aha/clockwork/CLAUDE.md Phase 1 +
            // /root/.claude/plans/cuddly-noodling-bumblebee.md Phase 4.
            // 2026-07-14: use distinctly-named wrapper Vars to sidestep the
            // .in()-vs-source Var namespace aliasing bug (see direction 4
            // writeup in /aha/clockwork/CLAUDE.md).
            Var wxi("wxi"), wyi("wyi"), wxo("wxo"), wyo("wyo");
            hw_output.in().compute_root()
                .tile(x, y, wxo, wyo, wxi, wyi, eff_mem_x, eff_mem_y)
                .hw_accelerate(wxi, wxo)
                .store_in(MemoryType::GLB);
            hw_output.compute_at(hw_output.in(), wxo);
            conv.compute_at(hw_output, Var::outermost());
            conv.update()
              .unroll(r.x, (int)unroll)
              .unroll(r.y, (int)unroll);
            hw_input.in().in().compute_at(hw_output, Var::outermost());
            hw_input.in().compute_at(hw_output.in(), wxo).store_in(MemoryType::GLB);
            hw_input.compute_root().accelerator_input();
            kernel.compute_at(hw_output, Var::outermost());

          } else if ((uint8_t)schedule == 4) {
            // 2-level nested Path B: wrapper tiled at MEM grain (outer,
            // accelerator-invocation boundary); source hw_output ALSO tiled at
            // sub-MEM grain within each invocation. Each accelerator call
            // computes (mem/sub_mem)^2 inner sub-tiles internally. Number of
            // accelerator invocations is unchanged from schedule=3 with the
            // same mem grain — this experiment sees whether internal
            // hierarchy affects per-invocation latency or resource use.
            Var wxi("wxi"), wyi("wyi"), wxo("wxo"), wyo("wyo");
            Var hxio("hxio"), hyio("hyio"), hxii("hxii"), hyii("hyii");
            hw_output.in().compute_root()
                .tile(x, y, wxo, wyo, wxi, wyi, eff_mem_x, eff_mem_y)
                .hw_accelerate(wxi, wxo)
                .store_in(MemoryType::GLB);
            hw_output
                .tile(x, y, hxio, hyio, hxii, hyii, eff_sub_mem_x, eff_sub_mem_y)
                .compute_at(hw_output.in(), wxo);
            conv.compute_at(hw_output, hxio);
            conv.update()
              .unroll(r.x, (int)unroll)
              .unroll(r.y, (int)unroll);
            hw_input.in().in().compute_at(hw_output, hxio);
            hw_input.in().compute_at(hw_output.in(), wxo).store_in(MemoryType::GLB);
            hw_input.compute_root().accelerator_input();
            kernel.compute_at(hw_output, hxio);

          } else { // schedule == 2
            // 3-level: host (whole image) -> GLB tiles -> MEM/CGRA tiles.
            // Pattern from apps/gaussian/gaussian_generator.cpp:171-231 (schedule 3 there).
            Var x_host, y_host, xi, yi, xo, yo, xii, yii, xio, yio;
            hw_output.in().in().compute_root()
                .tile(x, y, x_host, y_host, xo, yo, eff_glb_x, eff_glb_y);
            hw_output.in().compute_at(hw_output.in().in(), x_host)
                .tile(x, y, xo, yo, xi, yi, eff_glb_x, eff_glb_y)
                .hw_accelerate(xi, xo)
                .store_in(MemoryType::GLB);
            hw_output
                .tile(x, y, xio, yio, xii, yii, eff_mem_x, eff_mem_y)
                .compute_at(hw_output.in(), xo);
            conv.compute_at(hw_output, xio);
            conv.update()
              .unroll(r.x, (int)unroll)
              .unroll(r.y, (int)unroll);
            hw_input.in().in().compute_at(hw_output, xio);
            hw_input.in().compute_at(hw_output.in(), xo).store_in(MemoryType::GLB);
            hw_input.compute_root().accelerator_input();
            kernel.compute_at(hw_output, xio);
          }

        } else {  // schedule to CPU
          kernel.compute_at(output, x);
          
          conv.update()
            .unroll(r.x)
            .unroll(r.y);
        }

    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(ConvolutionKernel, conv_3_3)
