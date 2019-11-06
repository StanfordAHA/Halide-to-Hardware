#include <cstdio>
#include <chrono>

#include "conv_layer.h"
//#include "conv_layer_auto_schedule.h"

#include "halide_benchmark.h"
#include "HalideBuffer.h"

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main(int argc, char **argv) {
     //int Batch = 1;
    int Height = 64;
    int Width = 64;
    int Chin = 32;
    int Chout = 32;
    int Filter_sz = 3;

    //Buffer<int8_t> input(Width + Filter_sz - 1, Height + Filter_sz - 1, Chin, Batch);
    Buffer<int8_t> input(Width + Filter_sz - 1, Height + Filter_sz - 1, Chin);
    Buffer<int8_t> filter_dw(Filter_sz, Filter_sz, Chin);
    Buffer<int8_t> filter_pw(Chin, Chout);
    Buffer<int8_t> bias_dw(Chin);
    Buffer<int8_t> bias_pw(Chout);

    //for (int n = 0; n < input.dim(3).extent(); n++)
    for (int c = 0; c < input.channels(); c++)
    for (int h = 0; h < input.height(); h++)
    for (int w = 0; w < input.width(); w++) {
      //input(w, h, c, n) = (int8_t)rand();
      input(w, h, c) = (int8_t)rand();
    }

    for (int c = 0; c < filter_dw.channels(); c++)
    for (int y = 0; y < filter_dw.height(); y++)
    for (int x = 0; x < filter_dw.width(); x++) {
          filter_dw(x, y, c) = (int8_t)rand();
    }

    for (int k = 0; k < filter_pw.height(); k++)
    for (int c = 0; c < filter_pw.width(); c++) {
          filter_pw(c, k) = (int8_t)rand();
    }

    for (int x = 0; x < bias_dw.width(); x++) {
          bias_dw(x) = (int8_t)rand();
    }

    for (int x = 0; x < bias_pw.width(); x++) {
          bias_pw(x) = (int8_t)rand();
    }

    //Buffer<uint8_t> output(Width, Height, Chout, Batch);
    Buffer<uint8_t> output(Width, Height, Chout);

    printf("Start compile!\n");
    //conv_layer(input, filter_dw, filter_pw, bias_dw, bias_pw, output);
    conv_layer(input, output);
    printf("Finish compile, start running!\n");

    // Timing code

    // Manually-tuned version
    double min_t_manual = benchmark(1, 2, [&]() {
        //conv_layer(input, filter_dw, filter_pw, bias_dw, bias_pw, output);
        conv_layer(input, output);
    });
    printf("Manually-tuned time: %gms\n", min_t_manual * 1e3);

    // Auto-scheduled version
    // double min_t_auto = benchmark(10, 10, [&]() {
    //    conv_layer_auto_schedule(input, filter, output);
    // });
    // printf("Auto-scheduled time: %gms\n", min_t_auto * 1e3);

    return 0;
}
