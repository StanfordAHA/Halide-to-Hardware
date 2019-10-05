
#include "Halide.h"

#include "halide_image_io.h"
#include <stdio.h>
#include <iostream>

using namespace Halide::Tools;
using namespace std;

int main(int argc, char **argv) {

    Halide::Buffer<uint8_t> input = load_image("../../../../tutorial/images/rgb.png");
    cout << "Input rows = " << input.height() << endl;

    Halide::Func gradient;
    Halide::Var x, y;
    Halide::Expr e = x + y;

    gradient(x, y) = e;

    Halide::Buffer<int32_t> output = gradient.realize(800, 600);

    for (int j = 0; j < output.height(); j++) {
        for (int i = 0; i < output.width(); i++) {
            // We can access a pixel of an Buffer object using similar
            // syntax to defining and using functions.
            if (output(i, j) != i + j) {
                printf("Something went wrong!\n"
                       "Pixel %d, %d was supposed to be %d, but instead it's %d\n",
                       i, j, i+j, output(i, j));
                return -1;
            }
        }
    }

    printf("All tests passed!\n");

    return 0;
}
