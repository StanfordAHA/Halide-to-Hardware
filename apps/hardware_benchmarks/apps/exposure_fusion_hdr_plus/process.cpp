#include <cstdio>
#include "hardware_process_helper.h"
#include "halide_image_io.h"
#include <iostream>
#include <fstream>
#include <sstream>

#if defined(WITH_CPU)
   #include "exposure_fusion.h"
#endif

#if defined(WITH_COREIR)
    #include "coreir_interpret.h"
#endif

#if defined(WITH_CLOCKWORK)
    #include "rdai_api.h"
    #include "clockwork_sim_platform.h"
    #include "exposure_fusion_clockwork.h"
#endif

using namespace Halide::Tools;
using namespace Halide::Runtime;
using std::string;
using std::vector;

template<typename T>
T Clamp(const T& val, const T& min, const T& max) {
  return val > min ? (val < max ? val : max) : min;
}

int main( int argc, char **argv ) {
  std::map<std::string, std::function<void()>> functions;
  //OneInOneOut_ProcessController<uint16_t, uint8_t> processor("exposure_fusion");
  OneInOneOut_ProcessController<float, uint8_t> processor("exposure_fusion");

  #if defined(WITH_CPU)
      auto cpu_process = [&]( auto &proc ) {
        exposure_fusion( proc.input, proc.output );
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
          exposure_fusion_clockwork( proc.input, proc.output );
          RDAI_unregister_platform( rdai_platform );
        } else {
          printf("[RUN_INFO] failed to register RDAI platform!\n");
        }
      };
      functions["clockwork"] = [&](){ clockwork_process( processor ); };
  #endif

  // Add all defined functions
  processor.run_calls = functions;

  int ksize = 23;

  //processor.input = Buffer<uint8_t>(64, 64, 3);
  //processor.input = Buffer<uint8_t>(1096, 1112, 3);
  //processor.input = Buffer<uint16_t>(4000, 3000, 3);
  //processor.input = Buffer<float>(4000, 3000, 3);

  const int im_width = 1250;
  const int im_height = 1120;


  

  // If setting these to true, need to change the dimensions of the input buffer to match what's coming from CP
  // Need to output buffer to always have same dimensions as the input buffer 
  const int div_factor = 16; 
  const int width_pad = div_factor - ((int)(im_width) % int(div_factor));
  const int height_pad = div_factor - ((int)(im_height) % int(div_factor));

  const int padded_im_width = im_width + width_pad;
  const int padded_im_height = im_height + height_pad;

  bool use_CP_output = false; 
  bool x_max_printed = false;


  processor.input = Buffer<float>(padded_im_width, padded_im_height, 3);
  Buffer<float> unpadded_input;
  unpadded_input = Buffer<float>(im_width, im_height, 3);

  if(use_CP_output){
      processor.input = load_and_convert_image("../camera_pipeline_2x2/bin/output_cpu.png");
  } else {
     for (int color = 0; color < 3; color++){

        // Open the input file
        std::string kayvon_cp_output_file_prefix = "./pre_exposure_fusion_out_";
        //std::string taxi_10bit_filename = taxi_10bit_file_prefix + std::to_string(color) + "_small.txt";
        std::string kayvon_cp_output_filename = kayvon_cp_output_file_prefix + std::to_string(color) + ".txt";
        std::ifstream kayvon_cp_output_file(kayvon_cp_output_filename);

        // Check if the file is opened successfully
        int count = 0;
        if (!kayvon_cp_output_file.is_open()) {
            std::cerr << "Error: Unable to open file!" << std::endl;
            return 1; // Exit with error code
        }

        // Read the file line by line
        std::string line;
        int y = 0;

        while (std::getline(kayvon_cp_output_file, line)) {
        //while(count < im_width * im_height) {
            // Create a string stream from the current line
            std::istringstream iss(line);


            // Tokenize the line using ',' as the delimiter
            std::string token;
            int x = 0;
            while (std::getline(iss, token, ',')) {
            //while(x < im_width){
                //std::getline(iss, token, ',');
                count++;
                // convert this to unsigned short 
                //imgs(x, y, color) = static_cast<unsigned short>(stoul(token));
                //processor.input(x, y, color) = std::stof(token);
                unpadded_input(x, y, color) = std::stof(token);
                x++;
            }
            if(!x_max_printed){
              std::cout << "The maximum x value stored is: " << x << std::endl;
              x_max_printed = true;
            }
            y++;
        }

        // Close the input file
        kayvon_cp_output_file.close();

    }

    for (int y = 0; y < padded_im_height; y++){
      for (int x = 0; x < padded_im_width; x++){

        processor.input(x, y, 0) = unpadded_input(Clamp(x, 0, im_width-1), Clamp(y, 0, im_height-1), 0);
        processor.input(x, y, 1) = unpadded_input(Clamp(x, 0, im_width-1), Clamp(y, 0, im_height-1), 1);
        processor.input(x, y, 2) = unpadded_input(Clamp(x, 0, im_width-1), Clamp(y, 0, im_height-1), 2);

      }
    }





  }


  //processor.input = load_and_convert_image("../hdr_plus/bin/output_cpu.png");
  //processor.output = Buffer<uint8_t>(64-ksize+1, 64-ksize+1, 3);
  //processor.output = Buffer<uint8_t>(1096, 1112, 3);
  //processor.output = Buffer<uint8_t>(4000, 3000, 3);


  //processor.output = Buffer<uint8_t>(padded_im_width, padded_im_height, 3);
  processor.output = Buffer<uint8_t>(im_width, im_height, 3);
  //processor.output = Buffer<uint8_t>(padded_im_width, padded_im_height);
  

  processor.inputs_preset = true;
  auto cmd_output = processor.process_command(argc, argv);

  return cmd_output;
}
