#include <cstdio>
#include "hardware_process_helper.h"
#include "hardware_image_helpers.h"
#include "halide_image_io.h"

#if defined(WITH_CPU)
   #include "audio_pipeline.h"
#endif

#if defined(WITH_COREIR)
    #include "coreir_interpret.h"
#endif

#if defined(WITH_CLOCKWORK)
    #include "rdai_api.h"
    #include "clockwork_sim_platform.h"
    #include "audio_pipeline_clockwork.h"
#endif

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main( int argc, char **argv ) {
  std::map<std::string, std::function<void()>> functions;
  OneInOneOut_ProcessController<int16_t> processor("audio_pipeline");
  //OneInOneOut_ProcessController<int16_t> processor("audio_pipeline");

  #if defined(WITH_CPU)
      auto cpu_process = [&]( auto &proc ) {
        audio_pipeline( proc.input, proc.output );
      };
      functions["cpu"] = [&](){ cpu_process( processor ); } ;
  #endif
  
  #if defined(WITH_COREIR)
      auto coreir_process = [&]( auto &proc ) {
          run_coreir_on_interpreter<>( "bin/design_top.json",
                                       proc.input, proc.output,
                                       "self.in_arg_0_0_0", "self.out_0_0" );
      };
      functions["coreir"] = [&](){ coreir_process( processor ); };
  #endif
  
  #if defined(WITH_CLOCKWORK)
      auto clockwork_process = [&]( auto &proc ) {
        RDAI_Platform *rdai_platform = RDAI_register_platform( &rdai_clockwork_sim_ops );
        if ( rdai_platform ) {
          printf( "[RUN_INFO] found an RDAI platform\n" );
          audio_pipeline_clockwork( proc.input, proc.output );
          RDAI_unregister_platform( rdai_platform );
        } else {
          printf("[RUN_INFO] failed to register RDAI platform!\n");
        }
      };
      functions["clockwork"] = [&](){ clockwork_process( processor ); };
  #endif

  // Add all defined functions
  processor.run_calls = functions;

  // Read wave file
  AudioFile<int16_t> audioFile;

  // NOTE: Audio files are not consistent, hardware_image_helpers.h must have "junk"
  // in WAV_HEADER for some files. If changed, remake bin/process.
  // Both love.wav and distortion.wav has had the "junk" field removed.
  // NOTE: must change generator to have correct file length
  
  //std::string filename = "love";
  std::string filename = "distortion";
  
  int scheme = filename == "love" ? 2 : 1;
  readAudioData<int16_t>(filename + ".wav", &audioFile, scheme);
  std::cout << audioFile.header;

  processor.input   = audioFile.data;
  auto& in = processor.input;
  int ksize = 1;
  processor.output  = Buffer<int16_t>(in.dim(0).extent() - ksize + 1, in.dim(1).extent());
  processor.inputs_preset = true;
  
  //int overall_delay = 25;
  //processor.input   = Buffer<uint8_t>(100 + overall_delay);
  //for (int i=0; i<processor.input.dim(0).extent(); ++i) {
  //  processor.input(i) = i;
  //}
  //processor.inputs_preset = true;

  auto ret_value = processor.process_command(argc, argv);
  //std::cout << "input:" << std::endl;
  //for (int y=0; y<5; ++y) {
  //  for (int x=0; x<8; ++x) {
  //    std::cout << "y=" << y << ",x=" << x << " : " << std::hex << +processor.input(x, y) << std::endl;
  //  }
  //}
  //
  //std::cout << "output:" << std::endl;
  //for (int y=0; y<3; ++y) {
  //  for (int x=0; x<6; ++x) {
  //    std::cout << "y=" << y << ",x=" << x << " : " << std::hex << +processor.output(x, y, 0) << std::endl;
  //  }
  //}

  //testWavRead("distortion.wav");
  AudioFile<int16_t> outAudio;
  outAudio.header = audioFile.header;
  outAudio.data = processor.output;
  writeAudioData<int16_t>("bin/" + filename + "1.wav", outAudio, scheme);
  return ret_value;
  //return processor.process_command(argc, argv);
  
}
