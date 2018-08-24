#include <cstdio>
#include "halide_image_io.h"

enum ImageType {RANDOM, ASCENDING, UNIFORM};

void create_image(Halide::Runtime::Buffer<uint16_t>* input,
                  ImageType type = RANDOM,
                  int bias = 0);

void create_image(Halide::Runtime::Buffer<uint16_t>* input,
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

bool compare_images(const Halide::Runtime::Buffer<uint16_t>& image0,
                    const Halide::Runtime::Buffer<uint16_t>& image1) {
  bool equal_images = true;
  
  for (int y=0; y<image0.height(); y++) {
    for (int x=0; x<image0.height(); x++) {
      if(image0(x,y) != image1(x,y)) {
        equal_images = false;
      }
    }
  }

  return equal_images;
}

