#include <cstdio>
#include "halide_image_io.h"

enum ImageType {RANDOM, ASCENDING, UNIFORM, DIAGONAL, FLOATINGPOINT};

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

  case ImageType::DIAGONAL: {
    for (int y = 0; y < input->height(); y++) {
      for (int x = 0; x < input->width(); x++) {
        (*input)(x, y) = x + y;
      }
    }
    break;
  }
    
  case ImageType::FLOATINGPOINT: {
    int i = 1;
    for (int y = 0; y < input->height(); y++) {
      for (int x = 0; x < input->width(); x++) {
        float number = i + bias;
        uint16_t* pNumber = reinterpret_cast<uint16_t*>(&number);
        //std::cout << "number " << i << ": is " << std::hex << pNumber[0] << " then " << pNumber[1] << "," << pNumber[2] << "," << pNumber[3] << std::dec << "\n";
        (*input)(x, y) = pNumber[1];
        i++;
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
      image0.width() != image1.width() ||
      image0.channels() != image1.channels()) {
    std::cout << "Image sizes are not equal: "
              << "(" << image0.width() << "," << image0.height() << "," << image0.channels() << ") vs "
              << "(" << image1.width() << "," << image1.height() << "," << image1.channels() << ")\n";
    return 0;
  }
  
  for (int y=0; y<image0.height(); y++) {
    for (int x=0; x<image0.width(); x++) {
      for (int c=0; c<image0.channels(); c++) {
        if (image0(x,y,c) != image1(x,y,c) && equal_images) {
          int pixel_location = image0.channels()*image0.width()*y + image0.channels()*x + c;
          std::cout << "image0(y=" << y << ",x=" << x << ",c=" << c << ") = " << +image0(x,y,c)
                    << ", while "
                    << "image1(" << y << "," << x << "," << c << ") = " << +image1(x,y,c)
                    << std::hex << "  (debug at pixel " << +pixel_location << ")\n" << std::dec;
        }
        if (image0(x,y,c) != image1(x,y,c)) {
          equal_images = false;
        }
      }
    }
  }

  return equal_images;
}

