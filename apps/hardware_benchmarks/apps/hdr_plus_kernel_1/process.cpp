#include <cstdio>
#include "hardware_process_helper.h"
#include "halide_image_io.h"
#include "dirent.h"
#include <iostream>
#include <fstream>
#include <sstream>
//#include <libraw/libraw.h>
//#include <opencv4/opencv2/opencv.hpp>




#if defined(WITH_CPU)
   #include "hdr_plus_kernel_1.h"
#endif

#if defined(WITH_COREIR)
    #include "coreir_interpret.h"
#endif

#if defined(WITH_CLOCKWORK)
    #include "rdai_api.h"
    #include "clockwork_sim_platform.h"
    #include "hdr_plus_kernel_1_clockwork.h"
#endif

using namespace Halide::Tools;
using namespace Halide::Runtime;
using std::string;
using std::vector;

bool load_imgs(std::string dir_path, Buffer<uint16_t> &imgs) {
    std::vector<std::string> img_names;

    DIR* dir;
    struct dirent* ent;
    if ((dir = opendir(dir_path.c_str())) != NULL) {
        std::string file_name;
        std::string ext;
        while ((ent = readdir(dir)) != NULL) {
            file_name = std::string(ent->d_name);
            ext = Internal::get_lowercase_extension(file_name);
            if (ext != "pgm")
                continue;
            img_names.push_back(file_name);
        }
        std::sort(img_names.begin(), img_names.end());
        closedir(dir);
    } else {
      perror ("");
      return EXIT_FAILURE;
    }

    Buffer <uint16_t> img_tmp;
    //img_tmp =  Buffer <uint16_t>(im_width, im_height);
    /*printf("Right before calling load_image!\n");
    // Making this uint16_t so it operates on bayer raw 
    LibRaw rawProcessor;

    std::string file_path = dir_path + "/" + img_names[0]; 
    // Open the DNG file
    if (rawProcessor.open_file("./images/20171106_subset-bursts-0006_20160722_115157_431-payload_N000.dng") != LIBRAW_SUCCESS) {
        std::cerr << "Error: Cannot open DNG file" << std::endl;
        return 1;
    }

    printf("Succesfully opened DNG file!\n");

  // Decode the DNG file
    if (rawProcessor.unpack() != LIBRAW_SUCCESS) {
        std::cerr << "Error: Cannot unpack DNG file" << std::endl;
        return 1;
    }


    rawProcessor.raw2image();

    // Get image data
    int im_width = rawProcessor.imgdata.sizes.iwidth;
    int im_height = rawProcessor.imgdata.sizes.iheight;
    printf("The width and height are %d, %d\n", im_width, im_height);
    //FIXME!!!
    imgs = Buffer<uint16_t>(im_width, im_height, 1);

    printf("Right BEFORE copying buffer data!\n");
    // Copy image data to buffer
    for (int y = 0; y < im_height; ++y) {
        for (int x = 0; x < im_width; ++x) {
                imgs(x, y, 1) = rawProcessor.imgdata.image[y][x];
                //img_tmp(x, y) = 0;
                //printf("Data: %d", rawProcessor.imgdata.image[y][x]);
        }
    }

    printf("Right after copying buffer data!\n");

    // Close the DNG file
    rawProcessor.recycle();


    printf("Right after loading image");
    //exit(0);
    printf("Load image has been called!\n");
    return true;
    */
    //FIXME NOT A GOOD WAY OF DOING THINGS. FIX THIS!!!
    int num_imgs = img_names.size();
    // Getting rid of channel since we are operating on Bayer raw images: there isn't a channel (color) dimension yet 
    //int width, height, channel;
    int width, height;
    int ref = num_imgs/2;
    for (int i = ref; i > 0 ; i--) {
        std::iter_swap(img_names.begin() + i, img_names.begin() + i - 1);
    }

    for (int i = 0; i < num_imgs; i++) {
        img_tmp = load_image(dir_path + "/" + img_names[i]);
        if (i == 0) {
            width = img_tmp.width();
            height = img_tmp.height();
            //channel = img_tmp.channels();
            //imgs = Buffer<uint16_t>(width, height, channel, num_imgs);
            imgs = Buffer<uint16_t>(width, height, num_imgs);
            //printf("width: %d, height: %d, channel: %d, number of images: %d\n", 
            //        width, height, channel, num_imgs);
            printf("width: %d, height: %d, number of images: %d\n", 
                    width, height, num_imgs);
        }
        printf("loaded %s image\n", img_names[i].c_str());
        // changing this to slice along dimension 2 b/c operating on bayer raw 
        //imgs.sliced(3, i).copy_from(img_tmp);
        imgs.sliced(2, i).copy_from(img_tmp);
    }

    return true;
}

int main( int argc, char **argv ) {
  std::map<std::string, std::function<void()>> functions;
   OneInOneOut_ProcessController<int16_t, uint8_t> processor("hdr_plus_kernel_1");

  #if defined(WITH_CPU)
      auto cpu_process = [&]( auto &proc ) {
        hdr_plus_kernel_1( proc.input, proc.output );
      };
      functions["cpu"] = [&](){ cpu_process( processor ); } ;
  #endif
  
  #if defined(WITH_COREIR)
      auto coreir_process = [&]( auto &proc ) {
      };
      functions["coreir"] = [&](){ coreir_process( processor ); };
  #endif
  
  #if defined(WITH_CLOCKWORK)
      auto clockwork_process = [&]( auto &proc ) {
        RDAI_Platform *rdai_platform = RDAI_register_platform( &rdai_clockwork_sim_ops );
        if ( rdai_platform ) {
          printf( "[RUN_INFO] found an RDAI platform\n" );
          hdr_plus_kernel_1_clockwork( proc.input, proc.output );
          printf( "[RUN_INFO] finished running hdr_plus_kernel_1_clockwork\n" );
          RDAI_unregister_platform( rdai_platform );
        } else {
          printf("[RUN_INFO] failed to register RDAI platform!\n");
        }
      };
      functions["clockwork"] = [&](){ clockwork_process( processor ); };
  #endif

  // Add all defined functions
  processor.run_calls = functions;

 

/*
// BEGIN LOAD IMAGES FUNCTION
std::string dir_path = "./images/";
std::vector<std::string> img_names;

    DIR* dir;
    struct dirent* ent;
    if ((dir = opendir(dir_path.c_str())) != NULL) {
        std::string file_name;
        std::string ext;
        while ((ent = readdir(dir)) != NULL) {
            file_name = std::string(ent->d_name);
            ext = Internal::get_lowercase_extension(file_name);
            if (ext != "dng")
                continue;
            img_names.push_back(file_name);
        }
        std::sort(img_names.begin(), img_names.end());
        closedir(dir);
    } else {
      perror ("");
      return EXIT_FAILURE;
    }

    printf("Right before calling load_image!\n");
    // Making this uint16_t so it operates on bayer raw 
    LibRaw rawProcessor;

    std::string file_path = dir_path + "/" + img_names[0]; 
    // Open the DNG file
    if (rawProcessor.open_file("./images/20171106_subset-bursts-0006_20160722_115157_431-payload_N000.dng") != LIBRAW_SUCCESS) {
        std::cerr << "Error: Cannot open DNG file" << std::endl;
        return 1;
    }

    printf("Succesfully opened DNG file!\n");

  // Decode the DNG file
    if (rawProcessor.unpack() != LIBRAW_SUCCESS) {
        std::cerr << "Error: Cannot unpack DNG file" << std::endl;
        return 1;
    }


    //rawProcessor.raw2image();

    // Get image data
    int im_width = rawProcessor.imgdata.sizes.raw_width;
    int im_height = rawProcessor.imgdata.sizes.raw_height;
    printf("The width and height are %d, %d\n", im_width, im_height);
    //FIXME!!!
    imgs = Buffer<uint16_t>(im_width, im_height, 1);
    Buffer <uint16_t> img_tmp;
    img_tmp =  Buffer <uint16_t>(im_width, im_height);

    printf("Right BEFORE copying buffer data!\n");
    // Copy image data to buffer
    int raw_index;
    auto boosted = Buffer<uint16_t>(im_width, im_height);
    for (int y = 0; y < im_height; ++y) {
        for (int x = 0; x < im_width; ++x) {
                raw_index = x*y;
                imgs(x, y, 0) = rawProcessor.imgdata.rawdata.raw_image[raw_index];
                boosted(x, y) = rawProcessor.imgdata.rawdata.raw_image[raw_index] * 64;
                //img_tmp(x, y) = rawProcessor.imgdata.image[y][x];
                //printf("Data: %d  ", rawProcessor.imgdata.rawdata.raw_image[raw_index]);
        }
        //printf("\n");
    }
    save_image(boosted, "boosted_input.png");

 
    printf("Right after copying buffer data!\n");
    // Close the DNG file
    rawProcessor.recycle();
    printf("Right after loading image");
    //imgs = Buffer<uint16_t>(im_width, im_height, 1);
    //imgs.sliced(2, i).copy_from(img_tmp);
    //exit(0);
    printf("Load image has been called!\n");

// END LOAD IMAGES FUNCTION
*/



  // LOADING THE IMAGES IN AND CONCATENATING 
  //if(!load_imgs("./images/", imgs))
  //      return -1;


  // Load the input images (bayer raw)
  //Buffer<uint16_t> imgs;
  //Buffer<float> imgs;

  // Input is gPyramid[4]
  int im_width = 8;
  int im_height = 8;
  int num_frames = 3; 

  // Dims are tx, ty, xy, n
  processor.input = Buffer<int16_t>(im_width, im_height, 2, num_frames);
  //processor.input = Buffer<int16_t>(im_width, im_height, num_frames);


  for (int x = 0; x < im_width; x++) {
    for (int y = 0; y < im_height; y++){
      for (int xy = 0; xy < 2; xy++){
        for (int n = 0; n < num_frames; n++){
          processor.input(x, y, xy, n) = int16_t(0);
          //processor.input(x, y, n) = int16_t(0);
        }
      }
    }
  }
  
  processor.inputs_preset = true;
  printf("Successfully load input images!\n");




  // processor.output = Buffer<uint8_t>(39, 35, 3);
  //processor.output = Buffer<uint8_t>(256, 256, 3);
  //processor.output = Buffer<uint8_t>(32, 32);
  processor.output = Buffer<uint8_t>(16, 16);
  //processor.output = Buffer<uint8_t>(3, 3, 3);
  // processor.output = Buffer<uint8_t>(256, 256);
 // processor.output = Buffer<int16_t>(3, 3, 2, 3);

  auto cmd_output = processor.process_command(argc, argv);
  printf("Ran process command!");

  // //auto boosted_output = Buffer<uint16_t>(processor.output.dim(0).extent(), processor.output.dim(1).extent());
  // //auto output_2 = Buffer<uint16_t>(processor.output.dim(0).extent(), processor.output.dim(1).extent());
  // auto boosted_output = Buffer<uint8_t>(processor.output.dim(0).extent(), processor.output.dim(1).extent(), processor.output.dim(2).extent());
  // auto output_2 = Buffer<uint8_t>(processor.output.dim(0).extent(), processor.output.dim(1).extent(), processor.output.dim(2).extent());
  //   for (int y = 0; y < processor.output.dim(1).extent(); y++) {
  //     for (int x = 0; x < processor.output.dim(0).extent(); x++) {
  //       boosted_output(x, y) = processor.output(x, y) * 64;
  //       output_2(x, y) = processor.output(x, y);
  //     }
  //   }
  //   //save_image(boosted_output, "boosted_output.png");
  //   //save_image(output_2, "output_2.png");


  
  
  return cmd_output;
}