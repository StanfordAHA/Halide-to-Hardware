#include "Halide.h"
#include <tuple>
#include <unordered_map>

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

struct Tensor {
  Halide::Func f;
  std::vector<int> shape;
  std::string name;
  std::vector<Halide::Func> fs;
};

struct WeightShape {
  int c;  // output channels
  int w;
  int h;
  int pad;
  int stride;
};

std::ostream& operator<<(std::ostream& os, const std::vector<int>& vec);

class JitnetGenerator : public Halide::Generator<JitnetGenerator> {
public:
  Input<Buffer<float>> input{"input", 3};

  Input<Buffer<float>> conv1_weights{"conv1_weights", 4};
  Input<Buffer<float>> conv2_weights{"conv2_weights", 4};

  // set of four weights: 3x3, 1x3, 3x1, 1x1
  Input<Buffer<float>[4]> enc1_weights{"enc1_weights", 4};
  Input<Buffer<float>[4]> enc2_weights{"enc2_weights", 4};
  Input<Buffer<float>[4]> enc3_weights{"enc3_weights", 4};
  
  Input<Buffer<float>[4]> dec3_weights{"dec3_weights", 4};
  Input<Buffer<float>[4]> dec2_weights{"dec2_weights", 4};
  Input<Buffer<float>[4]> dec1_weights{"dec1_weights", 4};
  
  Input<Buffer<float>> conv3_weights{"conv3_weights", 4};
  Input<Buffer<float>> conv4_weights{"conv4_weights", 4};
  Input<Buffer<float>> conv5_weights{"conv5_weights", 4};

  // all 10 values for all relus
  Input<Buffer<float>[11]> conv_mu{"conv_mu", 1};
  Input<Buffer<float>[11]> conv_sigma{"conv_sigma", 1};

  // k_ic determines the number of input channels in a single xcel run
  GeneratorParam<int> k_ic{"k_ic", 4};    // default: 4
  
  // k_oc determines the number of output channels in a single xcel run
  GeneratorParam<int> k_oc{"k_oc", 3};    // default: 3
  
  Output<Buffer<float>> final_output{"final_output", 3};

  /** list out shapes of each layers weights **/
  // In channels inferred by input tensor shape
  // weight shapes: out channels, kernel_w, kernel_h, pad, stride.
  const WeightShape conv1_ws =  {8, 3, 3, 1, 2};
  const WeightShape conv2_ws =  {8, 3, 3, 1, 2};
                               
  const WeightShape enc1_ws =  {64, 3, 3, 1, 2};
  const WeightShape enc2_ws =  {64, 3, 3, 1, 2};
  const WeightShape enc3_ws = {128, 3, 3, 1, 2};
                               
  const WeightShape dec3_ws = {128, 3, 3, 1, 1};
  const WeightShape dec2_ws =  {64, 3, 3, 1, 1};
  const WeightShape dec1_ws =  {32, 3, 3, 1, 1};
  
  const WeightShape conv3_ws = {32, 3, 3, 1, 1};
  const WeightShape conv4_ws = {32, 3, 3, 1, 1};
  const WeightShape conv5_ws = {32, 1, 1, 0, 1};

  //const WeightShape conv1_ws =  {8, 3, 3, 0, 2};
  //const WeightShape conv2_ws =  {8, 3, 3, 0, 2};
  //
  //const WeightShape enc1_ws =  {64, 3, 3, 0, 2};
  //const WeightShape enc2_ws =  {64, 3, 3, 0, 2};
  //const WeightShape enc3_ws = {128, 3, 3, 0, 2};
  //
  //const WeightShape dec3_ws = {128, 3, 3, 0, 1};
  //const WeightShape dec2_ws =  {64, 3, 3, 0, 1};
  //const WeightShape dec1_ws =  {32, 3, 3, 0, 1};
  //
  //const WeightShape conv3_ws = {32, 3, 3, 0, 1};
  //const WeightShape conv4_ws = {32, 3, 3, 0, 1};
  //const WeightShape conv5_ws = {32, 1, 1, 0, 1};  

  Var c, i, j;

  void generate() {

    // Algorithm

    /** Declare arrays of other functions and build the requested block **/
    Tensor conv1, conv2;
    Tensor enc1, enc2, enc3;
    Tensor dec3, dec3_up, dec3_cat;
    Tensor dec2, dec2_cat, dec1;
    Tensor conv3, conv4, conv4_up, conv5;
      
    Tensor input_t;
    input_t.shape = {3, 1280, 720};
    input_t.f = input;
    input_t.name = "input";

    // initial convolutions
    conv1 = conv2D_block(input_t, conv1_ws, conv1_weights, conv_mu[0], conv_sigma[0], "conv1");
    conv2 = conv2D_block(conv1,   conv2_ws, conv2_weights, conv_mu[1], conv_sigma[1], "conv2");

    // encoding layers
    enc1 = coder_block(conv2, enc1_ws, enc1_weights, conv_mu[2], conv_sigma[2], "enc1");
    enc2 = coder_block(enc1,  enc2_ws, enc2_weights, conv_mu[3], conv_sigma[3], "enc2");
    enc3 = coder_block(enc2,  enc3_ws, enc3_weights, conv_mu[4], conv_sigma[4], "enc3");

    // decoding layers
    dec3 = coder_block(enc3, dec3_ws, dec3_weights, conv_mu[5], conv_sigma[5], "dec3");
    dec3_up = upsample_layer(dec3, "dec3_up", {dec3.shape[0], dec3.shape[1]*2, dec3.shape[2]*2-1}); // special case to not fully upsample height (23->45)
    //std::cout << "dec3: " << dec3_up.shape << " enc2: " << enc2.shape << std::endl;
    dec3_cat = concatenate_layer(dec3_up, enc2, "dec3_cat");
    
    dec2 = coder_block(dec3_cat, dec2_ws, dec2_weights, conv_mu[6], conv_sigma[6], "dec2", 2);
    //std::cout << "dec2: " << dec2.shape << " enc1: " << enc1.shape << std::endl;
    dec2_cat = concatenate_layer(dec2, enc1, "dec2_cat");
    
    dec1 = coder_block(dec2_cat, dec1_ws, dec1_weights, conv_mu[7], conv_sigma[7], "dec1", 4);

    // final convolutions with some upsampling
    conv3 = conv2D_block(dec1,  conv3_ws, conv3_weights, conv_mu[8], conv_sigma[8], "conv3");
    conv4 = conv2D_block(conv3, conv4_ws, conv4_weights, conv_mu[9], conv_sigma[9], "conv4", 2);
    //conv4_up = upsample_layer(conv4, "conv4_up");
    conv5 = conv2D_block(conv4, conv5_ws, conv5_weights, conv_mu[10], conv_sigma[10], "conv5");

    Func hw_output;
    hw_output = conv5.f;

    Var x, y;
    final_output(x, y, c) = hw_output(c, x, y);

    final_output.bound(c, 0, 32);
    final_output.bound(x, 0, 1280);
    final_output.bound(y, 0, 720);

    // TODO: Actually schedule this.
    if (get_target().has_feature(Target::Clockwork) || get_target().has_feature(Target::Pono)) {
      // loop order: r.z, r.x, r.y, xi, yi, xo, yo
      Var x_host,y_host, x_gb,y_gb, x_cgra,y_cgra;
      Var xi,yi;
      Var w_cgra, w_gb;
      Var z_cgra, z_gb;
      RVar rz_cgra, rz_gb;

      int schedule = 1;
      if (schedule == 1) {
        // Schedule: Create two accelerator calls.
        int imgsize = 64;
        auto gbsize = imgsize; // necessary until host can stream in tiles
        hw_output.compute_root();
        //hw_output.unroll(w, k_oc);
        hw_output
          //.tile(i, j, x_host,y_host, xi,yi, gbsize,gbsize)
          .tile(i, j, x_host,y_host, xi,yi, 1280,720) // all as one tile until host can stream tiles
          .reorder(xi,yi,c, x_host,y_host)
          .hw_accelerate(xi, x_host);

        //conv5.f.compute_at(final_output, Var::outermost());
        for (auto& f : conv4.fs) { f.compute_at(hw_output, x_host); }
        conv4.f.compute_at(hw_output, x_host);
        conv3.f.compute_at(hw_output, x_host);

        //conv3.f.stream_to_accelerator();

        auto hw_output2 = dec1.f;
        hw_output2.compute_root();
        hw_output2
          .tile(i, j, x_host,y_host, xi,yi, 640,360) // all as one tile until host can stream tiles
          .reorder(xi,yi,_0, x_host,y_host)
          .hw_accelerate(xi, x_host);

        for (auto& f : dec1.fs) { f.compute_at(hw_output2, x_host); }
        dec2.f.compute_at(hw_output2, x_host);
        for (auto& f : dec2.fs) { f.compute_at(hw_output2, x_host); }
        dec3.f.compute_at(hw_output2, x_host);

        enc3.f.compute_at(hw_output2, x_host);
        enc2.f.compute_at(hw_output2, x_host);
        enc1.f.compute_at(hw_output2, x_host);

        conv2.f.compute_at(hw_output2, x_host);
        conv1.f.compute_at(hw_output2, x_host);

        // stream inputs into the accelerator
        input.in().stream_to_accelerator();
      
        conv1_weights.in().stream_to_accelerator();
        conv2_weights.in().stream_to_accelerator();
        conv3_weights.in().stream_to_accelerator();
        conv4_weights.in().stream_to_accelerator();
        conv5_weights.in().stream_to_accelerator();

        for (size_t i=0; i<4; ++i) {
          enc1_weights[i].in().stream_to_accelerator();
          enc2_weights[i].in().stream_to_accelerator();
          enc3_weights[i].in().stream_to_accelerator();
          dec3_weights[i].in().stream_to_accelerator();
          dec2_weights[i].in().stream_to_accelerator();
          dec1_weights[i].in().stream_to_accelerator();
        }

        for (size_t i=0; i<11; ++i) {
          conv_mu[i].in().stream_to_accelerator();
          conv_sigma[i].in().stream_to_accelerator();
        }
        
      } else {
        int imgsize = 64;
        auto gbsize = imgsize; // necessary until host can stream in tiles
        hw_output.compute_root();
        //hw_output.unroll(w, k_oc);
        hw_output
          //.tile(i, j, x_host,y_host, xi,yi, gbsize,gbsize)
          .tile(i, j, x_host,y_host, xi,yi, 1280,720) // all as one tile until host can stream tiles
          .reorder(xi,yi,c, x_host,y_host)
          .hw_accelerate(xi, x_host);

        //conv5.f.compute_at(final_output, Var::outermost());
        for (auto& f : conv4.fs) { f.compute_at(hw_output, x_host); }
        conv4.f.compute_at(hw_output, x_host);
        conv3.f.compute_at(hw_output, x_host);

        dec1.f.compute_at(hw_output, x_host);
        for (auto& f : dec1.fs) { f.compute_at(hw_output, x_host); }
        dec2.f.compute_at(hw_output, x_host);
        for (auto& f : dec2.fs) { f.compute_at(hw_output, x_host); }
        dec3.f.compute_at(hw_output, x_host);

        enc3.f.compute_at(hw_output, x_host);
        enc2.f.compute_at(hw_output, x_host);
        enc1.f.compute_at(hw_output, x_host);

        conv2.f.compute_at(hw_output, x_host);
        conv1.f.compute_at(hw_output, x_host);

        // stream inputs into the accelerator
        input.in().stream_to_accelerator();
      
        conv1_weights.in().stream_to_accelerator();
        conv2_weights.in().stream_to_accelerator();
        conv3_weights.in().stream_to_accelerator();
        conv4_weights.in().stream_to_accelerator();
        conv5_weights.in().stream_to_accelerator();

        for (size_t i=0; i<4; ++i) {
          enc1_weights[i].in().stream_to_accelerator();
          enc2_weights[i].in().stream_to_accelerator();
          enc3_weights[i].in().stream_to_accelerator();
          dec3_weights[i].in().stream_to_accelerator();
          dec2_weights[i].in().stream_to_accelerator();
          dec1_weights[i].in().stream_to_accelerator();
        }

        for (size_t i=0; i<11; ++i) {
          conv_mu[i].in().stream_to_accelerator();
          conv_sigma[i].in().stream_to_accelerator();
        }
      }
      
    } else {  // schedule to CPU
      conv1.f.compute_root();
      conv2.f.compute_root();
      
      enc1.f.compute_root();
      enc2.f.compute_root();
      enc3.f.compute_root();
      
      dec3.f.compute_root();
      dec2.f.compute_root();
      dec1.f.compute_root();
      
      conv3.f.compute_root();
      conv4.f.compute_root();
      conv5.f.compute_root();
    }
      
  }

private:
  Func pad(Func f, Expr width, Expr height) {
    return Halide::BoundaryConditions::constant_exterior(f, 0.0f, {{0, width},{0,height}});
  }

  std::vector<int> compute_shape(const Tensor &in, const WeightShape &params) {
    int w = (1.0 / params.stride) * (params.pad * 2 + in.shape[1] - params.w + 1 + params.stride - 1);
    int h = (1.0 / params.stride) * (params.pad * 2 + in.shape[2] - params.h + 1 + params.stride - 1);
    int c = params.c;

    return {c, w, h};
  }

  Tensor conv2D(const Tensor &input, const WeightShape &weight_shape, const Func &weights, const std::string &name) {
    int p = weight_shape.pad;
    Func padded;
    // pad input
    if (p) {
      padded = pad(input.f, input.shape[1], input.shape[2]);
    } else {
      padded = input.f;
    }
    RDom r(0, input.shape[0], 0, weight_shape.w, 0, weight_shape.h);
    Func conv(name);
    conv(c, i, j) += weights(c, r.y, r.z, r.x) * padded(r.x, weight_shape.stride * i + r.y - p, weight_shape.stride * j + r.z - p);

    Tensor output;
    output.f = conv;
    output.name = name;
    output.shape = compute_shape(input, weight_shape);
    return output;
  }

  // assumes input is 3D (c, w, h) where w and h = 1
  Tensor fc_layer(const Tensor &input, const WeightShape &weight_shape, const Func &weights, const Func &bias, const std::string &name) {
    RDom r(0, input.shape[0]);
    Func fc(name);
    fc(c) = bias(c);
    fc(c) += weights(c, r.x) * input.f(r.x, 0, 0);

    Tensor output;
    output.f = fc;
    output.name = name;
    output.shape = compute_shape(input, weight_shape);

    return output;
  }

  Tensor relu_layer(const Tensor &input, const std::string &name) {
    Func relu(name);
    relu(c, i, j) = max(0.0f, input.f(c, i, j));
    Tensor output;
    output.f = relu;
    output.shape = input.shape;
    output.name = name;
    return output;
  }

  Tensor max_pool_layer(const Tensor &input, const WeightShape &weight_shape, const std::string &name) {
    int p = weight_shape.pad;
    Func padded;
    if (p) {
      padded = pad(input.f, input.shape[1], input.shape[2]);
    } else {
      padded = input.f;
    }
    RDom r(0, weight_shape.w, 0, weight_shape.h);
    Func pool(name);
    pool(c, i, j) = maximum(padded(c, weight_shape.stride * i + r.x - p, weight_shape.stride * j + r.y - p));
    Tensor output;
    output.f = pool;
    output.name = name;
    output.shape = compute_shape(input, weight_shape);

    return output;
  }

  Tensor avg_pool_layer(const Tensor &input, const WeightShape &weight_shape, const std::string &name) {
    int p = weight_shape.pad;
    Func padded;
    if (p) {
      padded = pad(input.f, input.shape[1], input.shape[2]);
    } else {
      padded = input.f;
    }
    RDom r(0, weight_shape.w, 0, weight_shape.h);
    float scale = weight_shape.w * weight_shape.h;
    Func pool(name);
    float n = 1.0f / scale;
    pool(c, i, j) += n * padded(c, weight_shape.stride * i + r.x - p, weight_shape.stride * j + r.y - p);

    Tensor output;
    output.f = pool;
    output.name = name;
    output.shape = compute_shape(input, weight_shape);

    return output;
  }

  Tensor norm_layer(const Tensor &input, const Func &mu, const Func &sigma, const std::string &name) {
    Func normed(name);
    Expr e = input.f(c, i, j);
    normed(c, i, j) = (input.f(c, i, j) - mu(c)) / (sqrt(sigma(c) + 1e-5f));
    Tensor output;
    output.f = normed;
    output.shape = input.shape;
    output.name = name;
    return output;
  }

  Tensor scale_layer(const Tensor &input, const Func &gamma, const Func &beta, const std::string &name) {
    Func scaled(name);
    scaled(c, i, j) = input.f(c, i, j) * gamma(c) + beta(c);
    Tensor output;
    output.f = scaled;
    output.shape = input.shape;
    output.name = name;
    return output;
  }

  Tensor sum_layer(const Tensor &t1, const Tensor &t2, const std::string &name) {
    assert(t1.shape == t2.shape);
    Func summed(name);
    summed(c, i, j) = t1.f(c, i, j) + t2.f(c, i, j);
    Tensor output;
    output.f = summed;
    output.shape = t1.shape;
    output.name = name;
    return output;
  }

  // Upsample using bilinear interpolation (1 3 3 1)
  Tensor upsample_layer(const Tensor &input, const std::string &name,
                        const std::vector<int> &shape = {}) {
    using Halide::_;
    Func upx, upy(name);
    Func f = input.f;
    upx(i, j, _) = 0.25f * f((i/2) - 1 + 2*(i % 2), j, _) + 0.75f * f(i/2, j, _);
    upy(i, j, _) = 0.25f * upx(i, (j/2) - 1 + 2*(j % 2), _) + 0.75f * upx(i, j/2, _);
    
    Tensor output;
    output.f = upy;
    if (shape.size() == 3) {
      output.shape = shape;
    } else {
      output.shape = input.shape;
      output.shape[1] *= 2; output.shape[2] *= 2;
    }
    output.name = name;
    return output;
  }

  Tensor conv2D_block(const Tensor &input, const WeightShape &weight_shape,
                      const Func &weights, const Func &mu, const Func &sigma,
                      const std::string &name, const int resize = 1) {
    Tensor conv = conv2D(input, weight_shape, weights, name);
    Tensor norm = norm_layer(conv, mu, sigma, name + "_norm");
    Tensor relu = relu_layer(norm, name + "_relu");
    
    Tensor output;
    output = relu;
    std::vector<Func> fs;
    assert(resize > 0 && ((resize & (resize-1)) == 0)); // resize must be a factor of 2, for now

    int upsample = resize;
    while (upsample > 1) {
      fs.push_back(output.f);
      output = upsample_layer(relu, name + "_up");
      upsample /= 2;
    }
    output.fs = fs;
    return output;
  }


  Tensor coder_block(const Tensor &input, const WeightShape &weight_shape,
                     const Func &convwh_weights, const Func &convh_weights,
                     const Func &convw_weights, const Func &convp_weights,
                     const Func &conv_mu, const Func &conv_sigma,
                     const std::string &name, const int resize = 1) {
    
    Tensor norm = norm_layer(input, conv_mu, conv_sigma, name + "_norm");
    Tensor relu = relu_layer(norm, name + "_relu");
    std::cout << name << " input shape: " << input.shape << std::endl;
    
    // conv3x3
    const WeightShape conv3x3_ws = {weight_shape.c, weight_shape.w, weight_shape.h,
                                    weight_shape.pad, weight_shape.stride};
    Tensor conv3x3 = conv2D(relu, conv3x3_ws, convwh_weights, name + "_conv3x3");
    //std::cout << "conv33 shape: " << conv3x3.shape << std::endl;
    
    // conv1x3
    const WeightShape conv1x3_ws = {weight_shape.c, 1, weight_shape.h,
                                    1, 1};
    Tensor conv1x3 = conv2D(conv3x3, conv1x3_ws, convh_weights, name + "_conv1x3");
    //std::cout << "conv13 shape: " << conv1x3.shape << std::endl;
    
    // conv3x1
    const WeightShape conv3x1_ws = {weight_shape.c, weight_shape.w, 1,
                                    0, 1};
    Tensor conv3x1 = conv2D(conv1x3, conv3x1_ws, convw_weights, name + "_conv3x1");
    //std::cout << "conv31 shape: " << conv3x1.shape << std::endl;
    
    // conv1x1
    const WeightShape conv1x1_ws = {weight_shape.c, 1, 1, 0, weight_shape.stride};
    Tensor conv1x1 = conv2D(input, conv1x1_ws, convp_weights, name + "_conv1x1");
    //std::cout << "conv11 shape: " << conv1x1.shape << std::endl;
    
    // sum_layer
    Tensor sum = sum_layer(conv3x1, conv1x1, name + "_sum");

    // upsample
    Tensor output = sum;
    int upsample = resize;
    std::vector<Func> fs;
    assert((upsample > 0) && ((upsample & (upsample-1)) == 0)); // check that it is a power of two
    while (upsample > 1) {
      fs.push_back(output.f);
      output = upsample_layer(output, name + "_up");
      upsample /= 2;
    }
    output.fs = fs;
    output.shape = compute_shape(input, weight_shape);
    return output;
  }

  Tensor coder_block(const Tensor &input, const WeightShape &weight_shape,
                     const Input<Buffer<float>[4]> &conv_weights, const Func &mu,
                     const Func &sigma, const std::string &name, const int resize = 1) {
    return coder_block(input, weight_shape, conv_weights[0], conv_weights[1],
                       conv_weights[2], conv_weights[3], mu, sigma, name, resize);
  }

  Tensor concatenate_layer(const Tensor &t1, const Tensor &t2, const std::string &name) {
    assert(t1.shape[1] == t2.shape[1]);
    assert(t1.shape[2] == t2.shape[2]);
    Func catted(name);
    int t1_oc = t1.shape[0];
    catted(c, i, j) = select(c < t1_oc,
                             t1.f(c, i, j),
                             t2.f(c - t1_oc, i, j));
    Tensor output;
    output.f = catted;
    output.shape = {t1_oc + t2.shape[0], t1.shape[1], t1.shape[2]};
    output.name = name;
    return output;
  }
  
  Func softmax_layer(const Tensor &input, const int classes, const std::string &name) {
    assert(input.shape[0] == classes);
    RDom r(0, classes);
    Func exp_vals;
    exp_vals(c) = exp(input.f(c));
    Func output(name);
    output(c) = exp_vals(c) / sum(exp_vals(r.x));
    return output;
  }
  
};

std::ostream& operator<<(std::ostream& os, const std::vector<int>& vec) {
  os << "[";
  for (size_t i=0; i<vec.size(); ++i) {
    os << vec.at(i);
    if (i < vec.size() - 1) {
      os << ",";
    }
  }
  os << "]";
  return os;
};

}  //namespace

HALIDE_REGISTER_GENERATOR(JitnetGenerator, jitnet)
