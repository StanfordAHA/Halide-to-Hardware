#ifndef HW_SUPPORT_HARDWARE_IMAGE_HELPERS_H
#define HW_SUPPORT_HARDWARE_IMAGE_HELPERS_H

#include <cstdio>
#include "halide_image_io.h"

#include <iostream>
#include <string>
#include <fstream>
#include <cstdint>

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
                    const Halide::Runtime::Buffer<T>& image1,
                    const int tolerance) {
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
      if (abs(image0(x,y) - image1(x,y)) > tolerance) {
        std::cout << "y=" << y << "," << "x=" << x
                  << " CPU val = "<< (int)image0(x,y) << "(" << std::hex << +image0(x,y) << ")" << std::dec
                  << ", Clock val = " << (int)image1(x,y) << "(" << std::hex << +image1(x,y) << ")" << std::dec << std::endl;
        equal_images = false;
      }
    }
  }

  return equal_images;
}

typedef struct WAV_HEADER {
    /* RIFF Chunk Descriptor */
    unsigned char   RIFF[4];        // RIFF Header Magic header
    uint32_t        FileSize;      // RIFF Chunk Size (overal file size in bytes)
    unsigned char   WAVE[4];        // WAVE Header
    /* "fmt" sub-chunk */
    unsigned char   fmt[4];         // FMT header
    uint32_t        FormatSize;     // Size of the fmt chunk
    uint16_t        AudioFormat;    // Audio format 1=PCM,1=IEEEfloat, 6=mulaw,7=alaw,     257=IBM Mu-Law, 258=IBM A-Law, 259=ADPCM
    uint16_t        NumOfChan;      // Number of channels 1=Mono 2=Sterio
    uint32_t        SamplesPerSec;  // Sampling Frequency in Hz
    uint32_t        bytesPerSec;    // bytes per second (SampleRate * NumChannels * BytesPerChannel)
    uint16_t        blockAlign;     // 2=16-bit mono, 4=16-bit stereo (NumChannels * BytesPerChannel)
    uint16_t        bitsPerChannel; // Number of bits per channel
    /* "data" sub-chunk */
  //uint16_t        junk; // for distortion file; love has none
    unsigned char   DataID[4];      // "data"  string
    uint32_t        DataSize;       // Sampled data length (in bytes)
} WaveHeader;

template <class T>
class AudioFile {
public:
  Halide::Runtime::Buffer<T> data;
  WaveHeader header;
};

inline std::ostream& operator<<(std::ostream& os, const WaveHeader& wavHeader) {
  os << "RIFF header                :" << wavHeader.RIFF[0] << wavHeader.RIFF[1] << wavHeader.RIFF[2] << wavHeader.RIFF[3] << std::endl;
  os << "WAVE header                :" << wavHeader.WAVE[0] << wavHeader.WAVE[1] << wavHeader.WAVE[2] << wavHeader.WAVE[3] << std::endl;
  os << "FMT                        :" << wavHeader.fmt[0] << wavHeader.fmt[1] << wavHeader.fmt[2] << wavHeader.fmt[3] << std::endl;
  os << "Format size                :" << wavHeader.FormatSize << std::endl;
  os << "Data size                  :" << wavHeader.FileSize << std::endl;

  // Display the sampling Rate from the header
  os << "Sampling Rate              :" << wavHeader.SamplesPerSec << std::endl;
  os << "Number of bits used        :" << wavHeader.bitsPerChannel << std::endl;
  os << "Number of channels         :" << wavHeader.NumOfChan << std::endl;
  os << "Number of bytes per second :" << wavHeader.bytesPerSec << std::endl;
  os << "Data length                :" << wavHeader.DataSize << std::endl;
  os << "Audio Format               :" << wavHeader.AudioFormat << std::endl;
  // Audio format 1=PCM,6=mulaw,7=alaw, 257=IBM Mu-Law, 258=IBM A-Law, 259=ADPCM

  os << "Block align                :" << wavHeader.blockAlign << std::endl;
  os << "Data string                :" << wavHeader.DataID[0] << wavHeader.DataID[1] << wavHeader.DataID[2] << wavHeader.DataID[3] << std::endl;
  //os << "Junk:                      :" << wavHeader.junk << std::endl;
  return os;
}

// return true if it verifies correctly
inline bool verifyWaveHeader(const WaveHeader& header) {
  assert(header.RIFF[0] == 'R' &&
         header.RIFF[1] == 'I' &&
         header.RIFF[2] == 'F' &&
         header.RIFF[3] == 'F');
  
  assert(header.WAVE[0] == 'W' &&
         header.WAVE[1] == 'A' &&
         header.WAVE[2] == 'V' &&
         header.WAVE[3] == 'E');

  assert(header.fmt[0] == 'f' &&
         header.fmt[1] == 'm' &&
         header.fmt[2] == 't' &&
         header.fmt[3] == ' ');

  bool DataID_matches =
    header.DataID[0] == 'd' &&
    header.DataID[1] == 'a' &&
    header.DataID[2] == 't' &&
    header.DataID[3] == 'a';
  if (!DataID_matches) { std::cout << "DataID does not match: 'data' != " << header.DataID << std::endl; }
  assert(DataID_matches);

  return true;
}

template <class T>
bool verifyAudioFile(const AudioFile<T>& audiofile) {
  return verifyWaveHeader(audiofile.header);
}

// find the file size
inline int getFileSize(FILE* inFile) {
    int fileSize = 0;
    fseek(inFile, 0, SEEK_END);

    fileSize = ftell(inFile);

    fseek(inFile, 0, SEEK_SET);
    return fileSize;
}

inline WaveHeader readWaveHeader(std::ifstream& infile) {
  WaveHeader wave_header;
  infile.read((char*)&wave_header, sizeof(wave_header));
  //infile >> wave_header;
  return wave_header;
}

inline void writeWaveHeader(std::ofstream& outfile, WaveHeader wave_header) {
  outfile.write((char*)&wave_header, sizeof(wave_header));
}

inline int testWavRead(std::string filename) {
    WaveHeader wavHeader;
    int headerSize = sizeof(WaveHeader), filelength = 0;

    const char* filePath = filename.c_str();
    std::cout << "Input wave file name: " << filePath << std::endl;

    FILE* wavFile = fopen(filePath, "r");
    if (wavFile == nullptr) {
        fprintf(stderr, "Unable to open wave file: %s\n", filePath);
        return 1;
    }

    //Read the header
    size_t bytesRead = fread(&wavHeader, 1, headerSize, wavFile);
    std::cout << "Header Read " << bytesRead << " bytes." << std::endl;
    if (bytesRead > 0) {
        //Read the data
        uint16_t bytesPerChannel = wavHeader.bitsPerChannel / 8;      //Number     of bytes per channel
        uint64_t numSamples = wavHeader.FileSize / bytesPerChannel; //How many samples are in the wav file?
        std::cout << "Calculated number of samples: " << numSamples << std::endl;
        static const uint16_t BUFFER_SIZE = 4096;
        int8_t* buffer = new int8_t[BUFFER_SIZE];
        while ((bytesRead = fread(buffer, sizeof buffer[0], BUFFER_SIZE / (sizeof buffer[0]), wavFile)) > 0) {
            /** DO SOMETHING WITH THE WAVE DATA HERE **/
            std::cout << "Read " << bytesRead << " bytes." << std::endl;
        }
        delete [] buffer;
        buffer = nullptr;
        filelength = getFileSize(wavFile);

        std::cout << "File is                    :" << filelength << " bytes." << std::endl;
        std::cout << wavHeader;
    }
    fclose(wavFile);
    return 0;
}

inline uint32_t getDataSize(const WaveHeader& wave_header, int scheme) {
  // 1: distortion
  // 2: love
  if (scheme == 1) { //distortion
    uint16_t HeaderSize = sizeof(wave_header);
    uint32_t DataSize = wave_header.FileSize - HeaderSize;
    return DataSize;
    
  } else if (scheme == 2) { //love
    uint32_t DataSize = wave_header.DataSize - wave_header.FormatSize;
    return DataSize;
    
  } else {
    return 0;
  }
}

template <class T>
int readAudioData(const std::string& filename, AudioFile<T>* audiofile, int scheme) {
  // Read and parse an audio file in WAVE format

  // open file
  std::ifstream infile;
  infile.open(filename.c_str(), std::ios::binary | std::ios::in);
  if (infile.fail()) {
    std::cout << "Unable to open wave file: " << filename << std::endl;
    return 1;
  }

  // Read the header data of the input WAVE file.
  WaveHeader wave_header = readWaveHeader(infile);
  verifyWaveHeader(wave_header);
  audiofile->header = wave_header;

  // Calculate data sizes.
  uint16_t bytesPerChannel = wave_header.bitsPerChannel / 8;
  assert(0 < bytesPerChannel && bytesPerChannel <= 4);
  assert(8*bytesPerChannel == wave_header.bitsPerChannel);


  uint32_t DataSize = getDataSize(wave_header, scheme);
  
  uint32_t num_samples = DataSize / (wave_header.NumOfChan * bytesPerChannel);
  uint32_t ten_percent = num_samples / 10;
  std::cout << "Data size  : " << DataSize << std::endl
            << " Product   : " << num_samples * wave_header.NumOfChan * bytesPerChannel << std::endl
            << "HeaderSize : " << sizeof(wave_header) << std::endl
            << "Num of chan: " << wave_header.NumOfChan << std::endl
            << "Bytes/chan : " << bytesPerChannel << std::endl
            << "NumSamples : " << num_samples << std::endl;
  assert(num_samples * wave_header.NumOfChan * bytesPerChannel == DataSize);
  
  if (num_samples > 1000000000) {
    std::cout << "sampleCount must be less than 1 million" << std::endl;
    return 2;
  }

  // Create storage for the audio data
  audiofile->data = Halide::Runtime::Buffer<T>(num_samples, wave_header.NumOfChan);

  // Read sample data by chunks, if PCM
  assert(wave_header.AudioFormat == 1); // Only PCM handled
  char data_buffer[bytesPerChannel];

  for (uint32_t sample = 0; sample < num_samples; sample++) {
    if (sample % ten_percent == 0) {
      std::cout << "Read " << sample / ten_percent * 10 << "%" << std::endl;
    }
    if (infile.fail()) {
      std::cout << "failed to read file at sample " << sample << std::endl;
      return 3;
    }
    
    T amplitude;    
    for (int ch = 0; ch < wave_header.NumOfChan; ch++) {
      // read signal amplitude
      infile.read(data_buffer, bytesPerChannel);
      // convert data from big endian to little
      // endian based on bytes in each channel sample
      switch (bytesPerChannel) {
      case 1:
        amplitude = data_buffer[0] & 0x00ff;
        amplitude -= 128; //in wave, 8-bit are unsigned, so shifting to signed
        break;
      case 2:
        amplitude =
          (data_buffer[0] & 0x00ff) |
          (data_buffer[1] << 8);
        break;
      case 3:
        amplitude =
          (data_buffer[0] & 0x00ff) |
          ((data_buffer[1] & 0x00ff) << 8) |
          (data_buffer[2] << 16);
        break;
      case 4:
        amplitude =
          (data_buffer[0] & 0x00ff) |
          ((data_buffer[1] & 0x00ff) << 8) |
          ((data_buffer[2] & 0x00ff) << 16) |
          (data_buffer[3] << 24);
        break;
      }
      audiofile->data(sample, ch) = amplitude;

      if (10000-10 < sample && sample < 10000) {
        //std::cout << "t=" << sample << ",c=" << ch << " : " << amplitude << std::endl;;
      }

    }
  }
  
  infile.close();
  return 0;
}

template <class T>
int writeAudioData(const std::string& filename, const AudioFile<T>& audiofile, int scheme) {
  // Write an audio file in WAVE format

  // open file
  std::ofstream outfile;
  outfile.open(filename.c_str(), std::ios::binary | std::ios::out);
  if (outfile.fail()) {
    std::cout << "Unable to open wave file: " << filename << std::endl;
    return 1;
  }

  // write the header
  verifyAudioFile(audiofile);
  writeWaveHeader(outfile, audiofile.header);
  WaveHeader wave_header = audiofile.header;

  uint16_t bytesPerChannel = wave_header.bitsPerChannel / 8;
  assert(0 < bytesPerChannel && bytesPerChannel <= 4);
  assert(8*bytesPerChannel == wave_header.bitsPerChannel);

  uint32_t DataSize = getDataSize(wave_header, scheme);
  
  uint32_t num_samples = DataSize / (wave_header.NumOfChan * bytesPerChannel);
  uint32_t ten_percent = num_samples / 10;
  std::cout << "Data size  : " << DataSize << std::endl
            << "Num of chan: " << wave_header.NumOfChan << std::endl
            << "Bytes/chan : " << bytesPerChannel << std::endl
            << "NumSamples : " << num_samples << std::endl;
  assert(num_samples * wave_header.NumOfChan * bytesPerChannel == DataSize);

  if (num_samples > 1000000000) {
    std::cout << "sampleCount must be less than 1 million" << std::endl;
    return 2;
  }

  // Only works if PCM
  assert(wave_header.AudioFormat == 1); // Only PCM handled

  for (uint32_t sample = 0; sample < num_samples; sample++) {
    if (sample % ten_percent == 0) {
      std::cout << "Wrote " << sample/ten_percent * 10 << "%" << std::endl;
    }
    if (outfile.fail()) {
      std::cout << "failed to write file at sample " << sample << std::endl;
      return 3;
    }
    
    //T amplitude;
    for (int ch = 0; ch < wave_header.NumOfChan; ch++) {
      // convert data from little endian to big
      // endian based on bytes in each channel sample
      T data = audiofile.data(sample, ch);
            
      //switch (bytesPerChannel) {
      //case 1:
      //  amplitude = data + 128; //in wave, 8-bit are unsigned, so shifting from signed
      //  amplitude = data & 0x00ff;
      //  break;
      //case 2:
      //  amplitude =
      //    ((data & 0x00ff) << 8) |
      //    ((data & 0xff00) >> 8);
      //  break;
      //case 3:
      //  amplitude =
      //    ((data & 0x0000ff) << 16) |
      //    ((data & 0x00ff00) << 0) |
      //    ((data & 0xff0000) >> 16);
      //  break;
      //case 4:
      //  amplitude =
      //    ((data & 0x000000ff) << 24) |
      //    ((data & 0x0000ff00) << 8) |
      //    ((data & 0x00ff0000) >> 8) |
      //    ((data & 0xff000000) >> 16);
      //  break;
      //}
      // write to file
      //outfile.write(reinterpret_cast<char*>(&amplitude), sizeof(amplitude));
      outfile.write(reinterpret_cast<char*>(&data), sizeof(data));

      if (10000-10 < sample && sample < 10000) {
        //std::cout << "t=" << sample << ",c=" << ch << " : " << data << std::endl;
      }

    }
  }
  
  outfile.close();
  return 0;
}

#endif
