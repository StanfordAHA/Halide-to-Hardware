#include <cstdio>

#include "conv_3_3/conv_3_3.h"
#include "conv_2_2/conv_2_2.h"
#include "conv_1_1/conv_1_1.h"
#include "down_sample/down_sample.h"
#include "up_sample/up_sample.h"

#include "hardware_process_helper.h"
#include "coreir_interpret.h"
#include "halide_image_io.h"

using namespace Halide::Tools;
using namespace Halide::Runtime;

inline Buffer<uint8_t> conv_conv(int *x, int *y, int k_x, int k_y, int c, Buffer<uint8_t> input, BUffer<uint8_t> kernel_0, Buffer<uint8_t> kernel_1) {
    *x = *x - k_x + 1;
    *y = *y - k_y + 1;
    Buffer<uint8+t> output_0(*x, *y, c);
    conv_3_3(input, kernel_0, output_0);

    *x = *x - k_x + 1;
    *y = *y - k_y + 1;
    Buffer<uint8_t> output_1(*x, *y, c);
    conv_3_3(output_0, kernel_1, output_1);
    return output_1;
}

inline Buffer<uint8_t> down(int *x, int *y, int k_x, int k_y, int c, Buffer<uint8_t> input) {
    *x = *x / 2;
    *y = *y / 2;
    Buffer<uint8_t> output(x, y, c);
    down_sample(input, output);
    return output;
}

inline Buffer<uint8_t> up_conv_cat(int *x, int *y, int k_x, int k_y, int c_0, int c_1, Buffer<uint8_t> input, Buffer<uint8_t> skip, Buffer<uint8_t> kernel) {
    *x = *x * 2;
    *y = *y * 2;
    Buffer<uint8_t> up(x, y, c_0);
    up_sample(input, up);

    Buffer<uint8_t> conv2x2(x, y, c_1);
    conv_2_2(up, kernel, conv2x2);

    int skip_x = skip.width();
    int skip_y = skip.height();

    int off_x = (skip_x - *x) / 2;
    int off_y = (skip_y - *y) / 2;

    Buffer<uint8_t> output(x, y, c_0);
    for (int i = 0; i < x; i++) {
        for (int j = 0; j < y; j++) {
            for (int c = 0; c < c_1; c++) {
                output(i, j, c) = skip(i + off_x, j + off_y, c);
                output(i, j, c + c_1) = conv_2_2(i, j, c);
            }
        }
    }

    return output;
}

int main(int argc, char **argv) {
    int x = 572;
    int y = 572;
    int k_x = 3;
    int k_y = 3;
    int up_x = 2;
    int up_y = 2;

    int c_0 = 64;
    int c_1 = 128;
    int c_2 = 256;
    int c_3 = 512;
    int c_4 = 1024;

    Buffer<uint8_t> image(x, y, 1); // input image

    Buffer<uint8_t> kernel_0_0(k_x, k_y, 1, c_0);
    Buffer<uint8_t> kernel_0_1(k_x, k_y, c_0, c_0);
    Buffer<uint8_t> output_0_pre = conv_conv(&x, &y, k_x, k_y, c_0, image, kernel_0_0, kernel_0_1);
    Buffer<uint8_t> output_0 = down(&x, &y, k_x, k_y, c_0, output_0_pre)

    /***********************************************************************/
    Buffer<uint8_t> kernel_1_0(k_x, k_y, c_0, c_1);
    Buffer<uint8_t> kernel_1_1(k_x, k_y, c_1, c_1);
    Buffer<uint8_t> output_1_pre = conv_conv(&x, &y, k_x, k_y, c_1, output_0, kernel_1_0, kernel_1_1);
    Buffer<uint8_t> output_1 = down(&x, &y, k_x, k_y, c_1, output_1_pre)

     /***********************************************************************/
    Buffer<uint8_t> kernel_2_0(k_x, k_y, c_1, c_2);
    Buffer<uint8_t> kernel_2_1(k_x, k_y, c_2, c_2);
    Buffer<uint8_t> output_2_pre = conv_conv(&x, &y, k_x, k_y, c_2, output_1, kernel_2_0, kernel_2_1);
    Buffer<uint8_t> output_2 = down(&x, &y, k_x, k_y, c_2, output_2_pre)

    /***********************************************************************/
    Buffer<uint8_t> kernel_3_0(k_x, k_y, c_2, c_3);
    Buffer<uint8_t> kernel_3_1(k_x, k_y, c_3, c_3);
    Buffer<uint8_t> output_3_pre = conv_conv(&x, &y, k_x, k_y, c_3, output_2, kernel_3_0, kernel_3_1);
    Buffer<uint8_t> output_3 = down(&x, &y, k_x, k_y, c_3, output_3_pre)

    /***********************************************************************/
    Buffer<uint8_t> kernel_4_0(k_x, k_y, c_3, c_4);
    Buffer<uint8_t> kernel_4_1(k_x, k_y, c_4, c_4);
    Buffer<uint8_t> output_4 = conv_conv(&x, &y, k_x, k_y, c4, output_3, kernel_4_0, kernel_4_1); 

    /***********************************************************************/
    /***********************************************************************/
    Buffer<uint8_t> rkernel_0_0(up_x, up_y, c_4, c_3);
    Buffer<uint8_t> up_0 = up_conv_cat(&x, &y, up_x, up_y, c_4, c_3, output_4, output_3, rkernel_0_0);
    Buffer<uint8_t> rkernel_0_1(k_x, k_y, c_4, c_3);
    Buffer<uint8_t> rkernel_0_2(k_x, k_y, c_3, c_3);
    Buffer<uint8_t> routput_0 = conv_conv(&x, &y, k_x, k_y, c_3, up_0, rkernel_0_1, rkernel_0_2);

    /***********************************************************************/
    Buffer<uint8_t> rkernel_1_0(up_x, up_y, c_3, c_2);
    Buffer<uint8_t> up_1 = up_conv_cat(&x, &y, up_x, up_y, c_3, c_2, routput_0, output_2, rkernel_1_0);
    Buffer<uint8_t> rkernel_1_1(k_x, k_y, c_3, c_2);
    Buffer<uint8_t> rkernel_1_2(k_x, k_y, c_2, c_2);
    Buffer<uint8_t> routput_1 = conv_conv(&x, &y, k_x, k_y, c_2, up_1, rkernel_1_1, rkernel_1_2);

    /***********************************************************************/
    Buffer<uint8_t> rkernel_2_0(up_x, up_y, c_2, c_1);
    Buffer<uint8_t> up_2 = up_conv_cat(&x, &y, up_x, up_y, c_2, c_1, routput_1, output_1, rkernel_2_0);
    Buffer<uint8_t> rkernel_2_1(k_x, k_y, c_2, c_1);
    Buffer<uint8_t> rkernel_2_2(k_x, k_y, c_1, c_1);
    Buffer<uint8_t> routput_2 = conv_conv(&x, &y, k_x, k_y, c_1, up_2, rkernel_2_1, rkernel_2_2);

    /***********************************************************************/
    Buffer<uint8_t> rkernel_3_0(up_x, up_y, c_1, c_0);
    Buffer<uint8_t> up_3 = up_conv_cat(&x, &y, up_x, up_y, c_1, c_0, routput_2, output_0, rkernel_3_0);
    Buffer<uint8_t> rkernel_3_1(k_x, k_y, c_1, c_0);
    Buffer<uint8_t> rkernel_3_2(k_x, k_y, c_0, c_0);
    Buffer<uint8_t> routput_3 = conv_conv(&x, &y, k_x, k_y, c_1, up_0, rkernel_3_1, rkernel_3_2);

    Buffer<uint8_t> last_kernel(1, 1, c_0, 2);
    Buffer<uint8_t> output(x, y, 2);
    conv_1_1(routput_3, last_kernel, output);
}
