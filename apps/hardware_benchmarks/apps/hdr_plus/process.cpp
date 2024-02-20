#include <cstdio>
#include "hardware_process_helper.h"
#include "halide_image_io.h"
#include "dirent.h"

#if defined(WITH_CPU)
   #include "hdr_plus.h"
#endif

#if defined(WITH_COREIR)
    #include "coreir_interpret.h"
#endif

#if defined(WITH_CLOCKWORK)
    #include "rdai_api.h"
    #include "clockwork_sim_platform.h"
    #include "hdr_plus_clockwork.h"
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
            if (ext != "png")
                continue;
            img_names.push_back(file_name);
        }
        std::sort(img_names.begin(), img_names.end());
        closedir(dir);
    } else {
      perror ("");
      return EXIT_FAILURE;
    }

    // Making this uint16_t so it operates on bayer raw 
    Buffer<uint16_t> img_tmp = load_image(dir_path + "/" + img_names[0]); 
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
  //OneInOneOut_ProcessController<uint16_t, uint8_t> processor("hdr_plus");
  //FIXME: FOR NOW, outputing int16_t. Once outputting RGB images, should send out uint8_t.
  OneInOneOut_ProcessController<uint16_t, uint16_t> processor("hdr_plus");

  #if defined(WITH_CPU)
      auto cpu_process = [&]( auto &proc ) {
        hdr_plus( proc.input, proc.output );
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
          hdr_plus_clockwork( proc.input, proc.output );
          RDAI_unregister_platform( rdai_platform );
        } else {
          printf("[RUN_INFO] failed to register RDAI platform!\n");
        }
      };
      functions["clockwork"] = [&](){ clockwork_process( processor ); };
  #endif

  // Add all defined functions
  processor.run_calls = functions;
  //processor.input  = Buffer<uint16_t>(iw, ih, num_frames_to_merge);
  //processor.output = Buffer<uint16_t>(ow, oh, num_frames_to_merge);

  // Load the input images (bayer raw) 
  //TODO: Think about fixing this argument here...
  Buffer<uint16_t> imgs;
  if(!load_imgs("./images/", imgs))
        return -1;
  
  int tile_x = 64;
  int tile_y = 64;

  processor.input = Buffer<uint16_t>(tile_x, tile_y, imgs.dim(2).extent());

  for (int im_num = 0; im_num < processor.input.dim(2).extent(); im_num++) {
    for (int y = 0; y < processor.input.dim(1).extent(); y++) {
        for (int x = 0; x < processor.input.dim(0).extent(); x++) {
            processor.input(x, y, im_num) = imgs(x, y, im_num);
        }
     }
  }
  
  //processor.input = imgs;
  processor.inputs_preset = true;
  printf("Successfully load input images!\n");

  // Placeholder: eventually, we want this to output a SINGLE bayer raw image after the merge step
  // FIXME: This also needs to become uint8_t after the merge step. Must change shape to remove 2 after merge step!
  processor.output = Buffer<uint16_t>(processor.input.dim(0).extent(), processor.input.dim(1).extent(), 2, processor.input.dim(2).extent());

  /*printf("Num arguments :%d\n", argc);
  printf("1st argument: %s\n", argv[0]);
  printf("2nd argument: %s\n", argv[1]);
  printf("3rd argument: %s\n", argv[2]);
  printf("4th argument: %s\n", argv[3]);*/
  auto cmd_output = processor.process_command(argc, argv);
  printf("Ran process command!");

  return cmd_output;
}
