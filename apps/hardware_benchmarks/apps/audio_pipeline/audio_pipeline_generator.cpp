/*
 * An application that performs a basic guitar pedal pipeline.
 * Stages of the pipeline are compression, distortion, noise
 * reduction, modulation, and reverberation.
 */

#include "Halide.h"

const float PI = 3.1415926536f;


namespace {

  using namespace Halide;
  using namespace Halide::ConciseCasts;

  int ksize = 1;
  float sample_freq = 44100;

//int32_t num_samples = 31467; //distortion
//int32_t num_samples = 412255; //love

  int num_samples = 20000; //general audio length to support
  int16_t max_value = 32767;

  Var t("t"), ti("ti"), to("to"), c("c");

  class AudioPipeline : public Halide::Generator<AudioPipeline> {
    
  public:
    Input<Buffer<int16_t>>  input{"input", 2}; // second dimension is each audio channel
    Output<Buffer<int16_t>> output{"output", 2};

    Func raw2float(Func input) {
      Func output("infloat");
      // new output range should be -1 to 1 floating point
      //output(t, c) = (f32(input(t, c)) / (float) max_value) * 2.f - 1.f;
      output(t, c) = (f32(input(t, c)) / (float) max_value);
      return output;
    }

    Func float2raw(Func input) {
      Func output("outint");
      //output(t, c) = u16(clamp((input(t, c) + 1.f) / 2.f * max_value, 0, max_value));
      output(t, c) = i16( clamp(input(t, c) * max_value, -max_value-1, max_value) );
      return output;
    }
    
    Func compress(Func input) {
      return input;
    }

    Func effects(Func input) {
      Func output("effected");
      
      int effect = 2;
      if (effect == 1) {
        // wah wah

      } else if (effect == 2) {
        // tremolo (low-frequency amplitude modulation)
        Func am, tremolo;
        float alpha = 0.5f;
        float mod_freq = 5;
        
        am(t, c) = 1 + alpha * sin(2*PI* mod_freq/sample_freq * t);
        tremolo(t, c) = am(t, c) * input(t, c);
        output(t, c) = tremolo(t, c);
          
      } else {
        // no effect
        output = input;
      }
      
      return output;
    }
    
    Func distort(Func input) {
      Func output("distorted");

      int effect = 2;
      if (effect == 1) {
        // Overdrive with soft clipping using non-linear soft saturation
        // 2x           for   0 ≤ x < 1/3
        // 1−(2−3x)^2/3 for 1/3 ≤ x < 2/3
        // 1            for 2/3 ≤ x ≤ 1
        Func overdrive;
        Expr x = input(t, c);
        Expr abs_x = abs(x);
        Expr lin = 2.f - 3.f * x;
        Expr alin = 2.f - 3.f * abs_x;
        //overdrive(t, c) = select(abs_x < 0.333f, 2.f * x,
        //                         abs_x < 0.666f, 1.f - lin*lin/3,
        //                         x     >=0.666f, 1.f,
        //                         -1.f);
        //output = overdrive;
        overdrive(t, c) = select(abs_x <  0.333f,   2.f * x,
                                 x     >  0.666f,   1.f,
                                 x     < -0.666f,  -1.f,
                                 x     >  0.333f,   1 - alin*alin / 3,
                                 /*x   < -0.333f*/ -1 + alin*alin / 3);
        output = overdrive;
        
      } else if (effect == 2) {
        // overdrive with hard clipping
        Func hard_overdrive;
        hard_overdrive(t, c) = clamp(3.f * input(t, c), -1.f, 1.f);

        output = hard_overdrive;

      } else if (effect == 3) {
        // overdrive with harsh clipping
        Func hard_overdrive;
        hard_overdrive(t, c) = clamp(2.f * input(t, c), 0.f, 1.f);

        output = hard_overdrive;
        
      } else {
        // no effect
        output = input;
      }
      return output;
    }
    
    Func noise_reduction(Func input) {
      // Noise gate: attack, release, hold times
      // https://www.mathworks.com/help/audio/ref/noisegate.html
      Func output;
      int threshold = -10; // allowable range: -140 to 0 dB
      float attack_time = 0.05; // allowable range: 0 to 4 s
      float release_time = 0.2; // allowable range: 0 to 4 s
      //float hold_time = 0.05; // allowable range: 0 to 4 s

      float threshold_lin = pow(10, threshold / 20.f);
      float alpha_attack = exp(-log(9) / (sample_freq * attack_time));
      float alpha_release = exp(-log(9) / (sample_freq * release_time));
      
      Func is_high, gain_comp;
      is_high(t, c) = abs(input(t, c)) > threshold_lin;
      gain_comp(t, c) = is_high(t, c);

      //Func gain, count_attack, count_release;
      //gain(t, c) = select(count_attack > hold_time, alpha_attack * gain(t-1) + (1-alpha_attack) * gain_comp(t, c),
      //                 count_attack <= hold_time, gain(t-1),
      //                 count_release > hold_time, alpha_release * gain(t-1) + (1-alpha_release) * gain_comp(t, c),
      //                 count_release <= hold_time, gain(t-1));
      Func gain;
      //gain(t, c) = 0.f;
      //gain(t, c) = select( is_high(t, c),
      //                  alpha_attack * gain(t-1) + (1-alpha_attack),
      //                  alpha_release * gain(t-1) );
      gain(t, c) = 1.f;
                       
      output(t, c) = gain(t, c) * input(t, c);
      
      return output;
    }
    
    Func modulation(Func input) {
      Func output;

      int effect = 0;
      if (effect == 1) {
        // flange effect
        int a = 0.5f; // attenuation factor
        int f = 1000; // flange frequency
        Func var_delay;
        var_delay(t, c) = sin(2*PI*f * t);
        
      } else {
        // no modulation effect
        output = input;
      }
      
      return output;
    }


    Func comb(Func input, int delay, int gain) {
      Func sum;
      sum(t, c) = gain * input(t, c) + input(t + delay, c);
      return sum;
    }

    Func allpass(Func input, int delay, int gain) {
      Func filter;
      filter(t, c) = input(t-delay) - (gain * input(t, c)) + (gain * filter(t-delay));
      return filter;
    }
    
    Func reverb(Func input) {
      Func output;

      int effect = 1;
      if (effect == 1) {
        // Schroeder reverb
        int decay_factor = 0.5f;
        int delay = 0.100f;
        auto comb1 = comb(input, delay, decay_factor);
        auto comb2 = comb(input, delay + 10.f, decay_factor);
        auto comb3 = comb(input, delay + 20.f, decay_factor);
        auto comb4 = comb(input, delay + 25.f, decay_factor);

        Func sum, allpass1, allpass2;
        sum(t, c) = comb1(t, c) + comb2(t, c) + comb3(t, c) + comb4(t, c);
        //allpass1 = allpass(sum, delay, decay_factor);
        //allpass2 = allpass(allpass1, delay, decay_factor);
        allpass2(t, c) = sum(t, c);

        output = allpass2;

      } else if (effect == 2) {
        // convolutional reverb
        
      } else {
        // no reverb
        output = input;
      }
      
      return output;
    }

    void generate() {

      Func hw_input;
      hw_input(t, c) = cast<int16_t>(input(t, c));

      Func mult_2;
      mult_2(t, c) = clamp(i32(hw_input(t, c)) * 2, 0, max_value);

      Func compressed;
      compressed = compress( raw2float(hw_input) );

      Func effect;
      effect = effects(compressed); // tremolo
      //effect = compress( compressed );

      Func distorted;
      distorted = float2raw( distort(effect) ); // overdrive

      Func redux;
      redux = noise_reduction(distorted);

      Func modulated;
      modulated = modulation(redux);

      Func final_audio;
      final_audio = reverb(modulated);

      Func hw_output;
      //hw_output(t, c) = final_audio(t, c);
      hw_output(t, c) = distorted(t, c);
      //hw_output(t, c) = i16(mult_2(t, c));
      output(t, c) = cast<int16_t>(hw_output(t, c));
            
      /* THE SCHEDULE */
      if (get_target().has_feature(Target::CoreIR)) {

      } else if (get_target().has_feature(Target::Clockwork)) {
        output.bound(t, 0, num_samples - ksize + 1);
        output.bound(c, 0, 2);

        hw_output.compute_root();

        hw_output.split(t, to, ti, num_samples - ksize + 1)
          //hw_output.split(t, to, ti, 10000)
          .reorder(ti, c, to)
          .hw_accelerate(ti, to);

        final_audio.compute_at(hw_output, to);
        modulated.compute_at(hw_output, to);
        redux.compute_at(hw_output, to);
        distorted.compute_at(hw_output, to);
        effect.compute_at(hw_output, to);
        compressed.compute_at(hw_output, to);

        hw_input.stream_to_accelerator();
          
      } else {    // schedule to CPU

      }
    }
  };

}  // namespace

HALIDE_REGISTER_GENERATOR(AudioPipeline, audio_pipeline)

