#include <cstdio>
#include "hardware_process_helper.h"
#include "halide_image_io.h"
#include <libraw/libraw.h>
#include "dirent.h"
#include <opencv4/opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
//#include <tiffio.h>

#if defined(WITH_CPU)
   #include "camera_pipeline_2x2.h"
#endif

#if defined(WITH_COREIR)
    #include "coreir_interpret.h"
#endif

#if defined(WITH_CLOCKWORK)
    #include "rdai_api.h"
    #include "clockwork_sim_platform.h"
    #include "camera_pipeline_2x2_clockwork.h"
#endif

using namespace Halide::Tools;
using namespace Halide::Runtime;
using std::string;
using std::vector;


bool load_img(std::string dir_path, Buffer<void> &img) {
    std::vector<std::string> img_names;

    DIR* dir;
    struct dirent* ent;
    if ((dir = opendir(dir_path.c_str())) != NULL) {
        std::string file_name;
        std::string ext;
        while ((ent = readdir(dir)) != NULL) {
            file_name = std::string(ent->d_name);
            ext = Internal::get_lowercase_extension(file_name);
            if (ext != "jpg")
                continue;
            img_names.push_back(file_name);
        }
        std::sort(img_names.begin(), img_names.end());
        closedir(dir);
    } else {
      perror ("");
      return EXIT_FAILURE;
    }

    //Buffer <uint16_t> img_tmp;
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

    printf("BEFORE LOAD IMAGE FUNCTION\n\n");
    std::cout << "Image name= " << img_names[0] << std::endl;
    img = load_image(dir_path + "/" + img_names[0]);
    printf("AFTER LOAD IMAGE FUNCTION\n\n"); 
    //exit(0);
    return true;
}


int main( int argc, char **argv ) {
  std::map<std::string, std::function<void()>> functions;
  //OneInOneOut_ProcessController<uint16_t, uint8_t> processor("camera_pipeline_2x2");
  ManyInOneOut_ProcessController<uint16_t, uint8_t> processor("camera_pipeline_2x2", {"input_image", "lens_shading_map"});

  #if defined(WITH_CPU)
      auto cpu_process = [&]( auto &proc ) {
        //camera_pipeline_2x2( proc.input, proc.output );
        camera_pipeline_2x2( proc.inputs["input_image"], proc.inputs["lens_shading_map"], proc.output );
      };
      functions["cpu"] = [&](){ cpu_process( processor ); } ;
  #endif
  
  #if defined(WITH_COREIR)
      auto coreir_process = [&]( auto &proc ) {
      };
      // Are we missing a run_coreir_on_interpreter call here? (E.g. see how it is done in exposure_fusion)
      functions["coreir"] = [&](){ coreir_process( processor ); };
  #endif
  
  #if defined(WITH_CLOCKWORK)
      auto clockwork_process = [&]( auto &proc ) {
        RDAI_Platform *rdai_platform = RDAI_register_platform( &rdai_clockwork_sim_ops );
        if ( rdai_platform ) {
          printf( "[RUN_INFO] found an RDAI platform\n" );
          //camera_pipeline_2x2_clockwork( proc.input, proc.output );
          camera_pipeline_2x2_clockwork( proc.inputs["input_image"], proc.inputs["lens_shading_map"], proc.output );
          RDAI_unregister_platform( rdai_platform );
        } else {
          printf("[RUN_INFO] failed to register RDAI platform!\n");
        }
      };
      functions["clockwork"] = [&](){ clockwork_process( processor ); };
  #endif

  // Add all defined functions
  processor.run_calls = functions;

  auto env_sch = getenv("schedule");
  auto schedule = env_sch ? atoi(env_sch) : 3;
  std::cout << "using schedule = " << schedule << std::endl;

  int output_tile_width  = 56;
  int output_tile_height = output_tile_width;
  
  //int input_width  = 1242;
  int host_tiling, glb_tiling;
  int ow, oh, iw, ih;
  switch (schedule) {
  case 1:
    processor.inputs_preset = true;
    host_tiling = 4;
    glb_tiling = 3;
    break;
  case 2:
  case 3:
    processor.inputs_preset = true;
    //host_tiling = 10;
    host_tiling = 1;
    glb_tiling = 1;
    //output_tile_width = 64-8;
    //output_tile_height = 64-8;
    //output_tile_width = 256-8;
    //output_tile_height = 192-8;
    output_tile_width = 256-8;
    output_tile_height = 192-8;
    break;
  default:
    processor.inputs_preset = false;
    host_tiling = 1;
    glb_tiling = 1;
    break;
  }

  int num_tiles          = host_tiling * glb_tiling;
  int output_width       = num_tiles * output_tile_width;
  int output_height      = num_tiles * output_tile_height;
  int blockSize = 9-1;

  // FIXME: why is this additional padding needed?
  vector<string> full_args(argv, argv + argc);
  string arch = full_args[2];

  iw = output_width + blockSize + 5*num_tiles;
  //iw = output_width * 2;
  ih = output_height + blockSize + 5*num_tiles;
  //ih = output_width * 2;

  if (schedule == 2 || schedule == 3) {
    ow = output_width;
    oh = output_height;
  //} else if (schedule == 1 && arch == "cpu") {
  //  ow = output_width + 8*num_tiles;
  //  oh = output_height + 8*num_tiles;
  //  iw = output_width + blockSize + 8*num_tiles;
  //  ih = output_height + blockSize + 8*num_tiles;
  } else if (schedule == 1) {
    ow = output_width + 2*num_tiles;
    oh = output_height + 2*num_tiles;
  } else {
    ow = output_width;
    oh = output_height;
  }


  std::cout << "Input size is : " << iw << " and " << ih << std::endl;
  std::cout << "Running with output size: " << output_width << "x" << output_height << std::endl
            << "  effective output size is " << ow << "x" << oh << std::endl;
  //processor.inputs["input_image"]  = Buffer<uint16_t>(output_width+blockSize-1, output_height+blockSize-1);
  //processor.inputs["input_image"]  = Buffer<uint16_t>(output_width+100+blockSize, output_height+100+blockSize);

  //processor.inputs["input_image"]  = Buffer<uint16_t>(iw, ih);
  processor.inputs["input_image"]  = Buffer<uint16_t>(1250, 1120);
  processor.inputs["lens_shading_map"] = Buffer<uint16_t>(4208, 3120);




    // BEGIN CODE FOR READING IN LENS SHADING FACTORS 


    // Open the input file
    std::ifstream lens_shading_file("lens_shading_factors.txt");

    // Check if the file is opened successfully
    int count = 0;
    if (!lens_shading_file.is_open()) {
        std::cerr << "Error: Unable to open file!" << std::endl;
        return 1; // Exit with error code
    }

    // Read the file line by line
    std::string line;
    int y = 0;

    while (std::getline(lens_shading_file, line)) {
        // Create a string stream from the current line
        std::istringstream iss(line);


        // Tokenize the line using ',' as the delimiter
        std::string token;
        int x = 0;
        while (std::getline(iss, token, ',')) {
            // Add token to lens_shading_map
            count++;
            processor.inputs["lens_shading_map"](x, y) = stoul(token);
            x++;
        }
        y++;
    }

    // Close the input file
    lens_shading_file.close();

    // END CODE FOR READING IN LENS SHADING FACTORS 

  printf("Total number of values written is %d\n", count);

  // for (int y = 0; y < processor.inputs["lens_shading_map"].dim(1).extent(); y++){
  //   for (int x = 0; x < processor.inputs["lens_shading_map"].dim(0).extent(); x++){
  //     printf("Factor: %d  ", uint16_t(processor.inputs["lens_shading_map"](x, y)));
  //     //std::cout << "Factor: " << processor.inputs["lens_shading_map"](x, y) << std::endl;
  //   }
  //   printf("\n");
  //     exit(0);
  // }

  printf("Dim 0 is %d\n", processor.inputs["lens_shading_map"].dim(0).extent());
  printf("Dim 1 is %d\n", processor.inputs["lens_shading_map"].dim(1).extent());

  //exit(0);
  
  // Now need to set the lens shading map
  


  //processor.output = Buffer<uint8_t>(output_width, output_height, 3);
  //processor.output = Buffer<uint8_t>(output_width+40, output_height+40, 3);
  //processor.output = Buffer<uint8_t>(ow, oh, 3);
  processor.output = Buffer<uint8_t>(1096, 1112, 3);
  //processor.output = Buffer<uint8_t>(2400, 1800, 3);
  


  Buffer<void> img;
  //if(!load_img("../hdr_plus/images/jpg/taxi/", img))
  //      return -1;

  printf("RIGHT BEFORE EQUALS SIGN\n\n");
  //processor.inputs["input_image"] = img;

  printf("GOT PAST EQUALS SIGN\n\n");
 
  //cv::Mat input_image = cv::imread("../hdr_plus/images/png/taxi/taxi_0.png", cv::IMREAD_UNCHANGED);
  int im_width = 1250;
  int im_height = 1120;
  auto input_image = Buffer<uint16_t>(im_width, im_height);
  //input_image = load_and_convert_image("../hdr_plus/bin/output_cpu.png");
  input_image = load_and_convert_image("../hdr_plus/output_2.png");



    // DO THIS SAME THING IN HDR+ PROCESS. OUTPUT THE IMAGE TWICE 
    // Loop over the elements of the cv::Mat matrix
    for (int y = 0; y < input_image.dim(1).extent(); ++y) {
        for (int x = 0; x < input_image.dim(0).extent(); ++x) {
            processor.inputs["input_image"](x, y) = input_image(x, y) * 1;
        }
    }

  if (schedule == 2 || schedule == 3) {
    // load this 2592x1968 image
    //std::cout << "Using a big dog image" << std::endl;
    //processor.inputs["input_image"] = load_and_convert_image("../../../images/bayer_raw.png");
    printf("Processor Input dim 0: %d\n", processor.inputs["input_image"].dim(0).extent());
    printf("Processor Input dim 1: %d\n", processor.inputs["input_image"].dim(1).extent());
    auto boosted = Buffer<uint16_t>(processor.inputs["input_image"].dim(0).extent(), processor.inputs["input_image"].dim(1).extent());
    for (int y = 0; y < processor.inputs["input_image"].dim(1).extent(); y++) {
      for (int x = 0; x < processor.inputs["input_image"].dim(0).extent(); x++) {
        boosted(x, y) = processor.inputs["input_image"](x, y) * 64;
      }
    }
    save_image(boosted, "boosted_input.png");
    
  } else {
    for (int y = 0; y < processor.inputs["input_image"].dim(1).extent(); y++) {
      for (int x = 0; x < processor.inputs["input_image"].dim(0).extent(); x++) {
        processor.inputs["input_image"](x, y) = x + y;
      }
    }
  }



  
  auto cmd_output = processor.process_command(argc, argv);


  int max = 0;
  for (int y=0; y<64; ++y) {
    for (int x=0; x<64; ++x) {
      max = max < processor.inputs["input_image"](x, y) ? processor.inputs["input_image"](x, y) : max;
    }
  }


  return cmd_output;
}
