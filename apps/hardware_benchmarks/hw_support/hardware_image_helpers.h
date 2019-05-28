#include <cstdio>
#include "halide_image_io.h"

enum ImageType {RANDOM, ASCENDING, UNIFORM};

template <typename T>
void create_image(Halide::Runtime::Buffer<T>* input,
                  ImageType type = RANDOM,
                  int bias = 0);

template <typename T>
void create_image(Halide::Runtime::Buffer<T>* input,
                  ImageType type,
                  int bias) {
  switch (type) {
  case ImageType::RANDOM: {
    for (int y = 0; y < input->height(); y++) {
      for (int x = 0; x < input->width(); x++) {
        (*input)(x, y) = rand() + bias;
      }
    }
    break;
  }

  case ImageType::ASCENDING: {
    int i = 1;
    for (int y = 0; y < input->height(); y++) {
      for (int x = 0; x < input->width(); x++) {
        (*input)(x, y) = i + bias;
        i++;
      }
    }
    break;
  }
      
  case ImageType::UNIFORM: {
    for (int y = 0; y < input->height(); y++) {
      for (int x = 0; x < input->width(); x++) {
        (*input)(x, y) = bias;
      }
    }
    break;
  }

  }
}

template <typename T>
bool compare_images(const Halide::Runtime::Buffer<T>& image0,
                    const Halide::Runtime::Buffer<T>& image1) {
  bool equal_images = true;

  if (image0.height() != image1.height() ||
      image0.width() != image1.width()) {
    std::cout << "Image sizes are not equal: "
              << "(" << image0.width() << "," << image0.height() << ") vs "
              << "(" << image1.width() << "," << image1.height() << ")\n";
    return 0;
  }
  
  for (int y=0; y<image0.height(); y++) {
    for (int x=0; x<image0.width(); x++) {
      if (image0(x,y) != image1(x,y) && equal_images) {
        int pixel_location = image0.width()*y + x;
        std::cout << "image0(y=" << y << ",x=" << x << ") = " << +image0(x,y)
                  << ", while image1(" << y << "," << x << ") = " << +image1(x,y)
                  << std::hex << "  (debug at pixel " << +pixel_location << ")\n" << std::dec;
      }
      if (image0(x,y) != image1(x,y)) {
        equal_images = false;
      }
    }
  }

  return equal_images;
}

