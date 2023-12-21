#include "Halide.h"
#include <tuple>
#include <unordered_map>

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;
using  std::to_string;

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

class MobilenetGenerator : public Halide::Generator<MobilenetGenerator> {
  // Number of repetitions of each block
  std::vector<int> nl = {2, 3, 4, 3, 3, 1};
  int total_layers = 16;
  
public:
  Input<Buffer<float>> input{"input", 3};

  /** parameter values for scaling layers **/
  Input<Buffer<float>[17]> bottle_p1{ "bottle_p1", 4 };
  Input<Buffer<float>[17]> bottle_dw{ "bottle_dw", 4 };
  Input<Buffer<float>[17]> bottle_p2{ "bottle_p2", 4 };
  Input<Buffer<float>[17]> bottle_mu1{ "bottle_mu1", 1 };
  Input<Buffer<float>[17]> bottle_mu2{ "bottle_mu2", 1 };
  Input<Buffer<float>[17]> bottle_mu3{ "bottle_mu3", 1 };
  Input<Buffer<float>[17]> bottle_sigma1 { "bottle_sigma1", 1 };
  Input<Buffer<float>[17]> bottle_sigma2 { "bottle_sigma2", 1 };
  Input<Buffer<float>[17]> bottle_sigma3 { "bottle_sigma3", 1 };

  Input<Buffer<float>[3]> conv_weights{"conv_weights", 4};
  Input<Buffer<float>[3]> conv_mu{"conv_mu", 1};
  Input<Buffer<float>[3]> conv_sigma{"conv_sigma", 1};

  // k_ic determines the number of input channels in a single xcel run
  GeneratorParam<int> k_ic{"k_ic", 4};    // default: 4
  
  // k_oc determines the number of output channels in a single xcel run
  GeneratorParam<int> k_oc{"k_oc", 3};    // default: 3
  
  Output<Buffer<float>> final_output{"final_output", 3};

  /** list out shapes of each layers weights **/
  // In channels inferred by input tensor shape
  // weight shapes: out channels, kernel_w, kernel_h, pad, stride.
  const WeightShape conv1_ws = {32, 3, 3, 1, 2};
  
  const WeightShape bottle1_ws = {16, 3, 3, 1, 1};
  
  const WeightShape bottle2_init_ws = {24, 3, 3, 1, 2};
  const WeightShape bottle2_rest_ws = {24, 3, 3, 1, 1};
  
  const WeightShape bottle3_init_ws = {32, 3, 3, 1, 2};
  const WeightShape bottle3_rest_ws = {32, 3, 3, 1, 1};
  
  const WeightShape bottle4_init_ws = {64, 3, 3, 1, 2};
  const WeightShape bottle4_rest_ws = {64, 3, 3, 1, 1};
  
  const WeightShape bottle5_init_ws = {96, 3, 3, 1, 1};
  const WeightShape bottle5_rest_ws = {96, 3, 3, 1, 1};
  
  const WeightShape bottle6_init_ws = {160, 3, 3, 1, 2};
  const WeightShape bottle6_rest_ws = {160, 3, 3, 1, 1};
  
  const WeightShape bottle7_init_ws = {320, 3, 3, 1, 1};
  const WeightShape bottle7_rest_ws = {320, 3, 3, 1, 1};

  const WeightShape conv2_ws =   {1280, 1, 1, 0, 1};
  const WeightShape avgpool_ws = {1280, 7, 7, 0, 1};
  const WeightShape conv3_ws =   {1280, 1, 1, 0, 1};

  Var c, i, j;

  void generate() {

    Func conv1_weights[3] = {conv_weights[0], conv_mu[0], conv_sigma[0]};
    std::vector<Func> weights1 = {bottle_p1[16], bottle_dw[16], bottle_p2[16],
                                  bottle_mu1[16], bottle_mu2[16], bottle_mu3[16],
                                  bottle_sigma1[16], bottle_sigma2[16], bottle_sigma3[16]};

    // indexed by layer index, iteration count, weight group of 9
    std::vector<std::vector<std::vector<Func>>> weights;
    
    Func conv2_weights[3] = {conv_weights[1], conv_mu[1], conv_sigma[1]};
    Func conv3_weights[3] = {conv_weights[2], conv_mu[2], conv_sigma[2]};
    //std::cout << "let's make some weights" << std::endl;

    // Algorithm
    /** Reorganize the weights for each of the layers **/
    for (size_t set=0, index=0; set<nl.size(); ++set) {
      weights.emplace_back(std::vector<std::vector<Func>>(nl[set]));

      for (int layer=0; layer<nl[set]; ++layer, ++index) {
        //std::cout << "created weights for set=" << set << ", layer=" << layer << ", index=" << index << std::endl;
        std::vector<Func> weight_set = {bottle_p1[index], bottle_dw[index], bottle_p2[index],
                                        bottle_mu1[index], bottle_mu2[index], bottle_mu3[index],
                                        bottle_sigma1[index], bottle_sigma2[index], bottle_sigma3[index]};
        //weights[set].emplace_back(weight_set);
        weights[set][layer] = weight_set;
      }
    }
    
    auto& weights2 = weights[0];
    //std::cout << "weights2.size=" << weights2.size() << " s=" << weights2[0].size() << std::endl;
    auto& weights3 = weights[1];
    auto& weights4 = weights[2];
    auto& weights5 = weights[3];
    auto& weights6 = weights[4];
    auto& weights7 = weights[5];
    std::cout << " weights set" << std::endl;

    /** Declare arrays of other functions and build the requested block **/
    Tensor conv1;
    Tensor bottleneck1;
    
    Tensor bottleneck2[nl[0]];
    Tensor bottleneck3[nl[1]];
    Tensor bottleneck4[nl[2]];
    Tensor bottleneck5[nl[3]];
    Tensor bottleneck6[nl[4]];
    Tensor bottleneck7[nl[5]];
    
    Tensor conv2;
    Tensor avgpool;
    Tensor conv3;

    // these tensors are different depending on the block and must be conditionally assigned.
    Tensor input_t;
    input_t.name = "input";
    input_t.shape = {3, 224, 224};
    input_t.f = input;

    conv1 = conv_bn_relu(input_t, conv1_ws, conv1_weights, "conv1");
    bottleneck1 = bottleneck_block(conv1, /*k=*/1, bottle1_ws, weights1, "bottle1");
    
    for (int i=0; i<nl[0]; ++i) {
      bottleneck2[i] = i==0 ?
        bottleneck_block(bottleneck1,      6, bottle2_init_ws, weights2[i], "bottle2_" + to_string(i)) :
        bottleneck_block(bottleneck2[i-1], 1, bottle2_rest_ws, weights2[i], "bottle2_" + to_string(i)); }
    for (int i=0; i<nl[1]; ++i) {
        bottleneck3[i] = i==0 ?
          bottleneck_block(bottleneck2[nl[0]-1], 6, bottle3_init_ws, weights3[i], "bottle3_" + to_string(i)) :
          bottleneck_block(bottleneck3[i-1],     1, bottle3_rest_ws, weights3[i], "bottle3_" + to_string(i)); }
    for (int i=0; i<nl[2]; ++i) {
        bottleneck4[i] = i==0 ?
          bottleneck_block(bottleneck3[nl[1]-1], 6, bottle4_init_ws, weights4[i], "bottle4_" + to_string(i)) :
          bottleneck_block(bottleneck4[i-1],     1, bottle4_rest_ws, weights4[i], "bottle4_" + to_string(i)); }
    for (int i=0; i<nl[3]; ++i) {
        bottleneck5[i] = i==0 ?
          bottleneck_block(bottleneck4[nl[2]-1], 6, bottle5_init_ws, weights5[i], "bottle5_" + to_string(i)) :
          bottleneck_block(bottleneck5[i-1],     1, bottle5_rest_ws, weights5[i], "bottle5_" + to_string(i)); }
    for (int i=0; i<nl[4]; ++i) {
        bottleneck6[i] = i==0 ?
          bottleneck_block(bottleneck5[nl[3]-1], 6, bottle6_init_ws, weights6[i], "bottle6_" + to_string(i)) :
          bottleneck_block(bottleneck6[i-1],     1, bottle6_rest_ws, weights6[i], "bottle6_" + to_string(i)); }
    for (int i=0; i<nl[5]; ++i) {
        bottleneck7[i] = i==0 ?
          bottleneck_block(bottleneck6[nl[4]-1], 6, bottle7_init_ws, weights7[i], "bottle7_" + to_string(i)) :
          bottleneck_block(bottleneck7[i-1],     1, bottle7_rest_ws, weights7[i], "bottle7_" + to_string(i)); }

    conv2 = conv_bn_relu(bottleneck7[nl[5]-1], conv2_ws, conv2_weights, "conv2");
    avgpool = avg_pool_layer(conv2, avgpool_ws, "avgpool");
    conv3 = conv_bn_relu(avgpool, conv3_ws, conv3_weights, "finalconv");

    final_output = conv3.f;
    
    final_output.bound(c, 0, 1000);

    //final_output.bound(c, 0, 32);
    //final_output.bound(x, 0, 1280);
    //final_output.bound(y, 0, 720);

    // TODO: Actually schedule this.
    if (get_target().has_feature(Target::Clockwork) || get_target().has_feature(Target::Pono)) {
      // loop order: r.z, r.x, r.y, xi, yi, xo, yo
      Var x_host,y_host, x_gb,y_gb, x_cgra,y_cgra;
      Var xi,yi;
      Var w_cgra, w_gb;
      Var z_cgra, z_gb;
      RVar rz_cgra, rz_gb;

      int schedule = 0;
      if (schedule == 1) {
        // Schedule: Create two accelerator calls.
        
      } else {
        int imgsize = 64;
        auto gbsize = imgsize; // necessary until host can stream in tiles

        // start tiling after last conv, before final 3 layers
        auto hw_output = conv3.f;
        hw_output.compute_root();
        //hw_output.unroll(w, k_oc);
        hw_output
          //.tile(i, j, x_host,y_host, xi,yi, gbsize,gbsize)
          .tile(i, j, x_host,y_host, xi,yi, 1280,720) // all as one tile until host can stream tiles
          .reorder(xi,yi,c, x_host,y_host)
          .hw_accelerate(xi, x_host);

        // buffer all layers
        for (auto& f : conv1.fs) { f.compute_at(hw_output, x_host); }
        for (auto& f : bottleneck1.fs) { f.compute_at(hw_output, x_host); }
        
        for (auto& bottleneck : bottleneck2) {
          for (auto& f : bottleneck.fs) { f.compute_at(hw_output, x_host); } }
        
        for (auto& bottleneck : bottleneck3) {
          for (auto& f : bottleneck.fs) { f.compute_at(hw_output, x_host); } }
        
        for (auto& bottleneck : bottleneck4) {
          for (auto& f : bottleneck.fs) { f.compute_at(hw_output, x_host); } }
        
        for (auto& bottleneck : bottleneck5) {
          for (auto& f : bottleneck.fs) { f.compute_at(hw_output, x_host); } }
        
        for (auto& bottleneck : bottleneck6) {
          for (auto& f : bottleneck.fs) { f.compute_at(hw_output, x_host); } }
        
        for (auto& bottleneck : bottleneck7) {
          for (auto& f : bottleneck.fs) { f.compute_at(hw_output, x_host); } }

        for (auto& f : conv2.fs) { f.compute_at(hw_output, x_host); }
        for (auto& f : conv3.fs) { f.compute_at(hw_output, x_host); }


        // stream inputs into the accelerator
        input.in().stream_to_accelerator();
      
        for (size_t i=0; i<17; ++i) {
          bottle_p1[i].in().stream_to_accelerator();
          bottle_dw[i].in().stream_to_accelerator();
          bottle_p2[i].in().stream_to_accelerator();
          bottle_mu1[i].in().stream_to_accelerator();
          bottle_mu2[i].in().stream_to_accelerator();
          bottle_mu3[i].in().stream_to_accelerator();
          bottle_sigma1[i].in().stream_to_accelerator();
          bottle_sigma2[i].in().stream_to_accelerator();
          bottle_sigma3[i].in().stream_to_accelerator();
        }

        for (size_t i=0; i<3; ++i) {
          conv_weights[i].in().stream_to_accelerator();
          conv_mu[i].in().stream_to_accelerator();
          conv_sigma[i].in().stream_to_accelerator();
        }

      }
      
    } else {  // schedule to CPU

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

  Tensor conv2dw(const Tensor &input, const WeightShape &weight_shape, const Func &weights_dw, const Func &weights_pw, const std::string &name) {
    int p = weight_shape.pad;
    Func padded;
    // pad input
    if (p) {
      padded = pad(input.f, input.shape[1], input.shape[2]);
    } else {
      padded = input.f;
    }
    RDom r(0, input.shape[0], 0, weight_shape.w, 0, weight_shape.h);
    Func conv_dw(name);
    conv_dw(c, i, j) += weights_dw(c, r.y, r.z) * padded(c, weight_shape.stride * i + r.y - p, weight_shape.stride * j + r.z - p);

    Var k;
    Func conv_pw(name);
    conv_pw(k, i, j) += weights_pw(k, r.x) * conv_dw(r.x, i, j);

    Tensor output;
    output.f = conv_pw;
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

  Tensor relu6_layer(const Tensor &input, const std::string &name) {
    Func relu(name);
    relu(c, i, j) = clamp(input.f(c, i, j), 0.0f, 6.0f);
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
    if (t1.shape != t2.shape) {
      std::cout << "t1=" << t1.shape << std::endl
                << "t2=" << t2.shape << std::endl;
    }
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
    output.shape = compute_shape(input, weight_shape);
    return output;
  }

  Tensor conv_bn_relu(const Tensor &input, const WeightShape &weight_shape,
                      const Func &weights, const Func &mu, const Func &sigma,
                      const std::string &name) {
    return conv2D_block(input, weight_shape, weights, mu, sigma, name);
  }

  // weights should be {weights, mu, sigma}
  Tensor conv_bn_relu(const Tensor &input, const WeightShape &weight_shape,
                      const Func weights[3], const std::string &name) {
    return conv2D_block(input, weight_shape, weights[0], weights[1], weights[2], name);
  }

  
  Tensor bottleneck_block(const Tensor &input, const int expansion, const WeightShape &weight_shape,
                          const Func &convp1_weights, const Func &convdw_weights, const Func &convp2_weights, 
                          const Func conv_mu[3], const Func conv_sigma[3],
                          const std::string &name) {
    std::vector<Func> fs;
    int k = expansion;
    std::cout << name << " input shape: " << input.shape << std::endl;
    
    // conv_pointwise1
    const WeightShape conv_pw1_ws = {input.shape[0]*k, /*width=*/1, /*height=*/1,
                                     /*pad=*/0, /*stride=*/1};
    Tensor conv_pw1 = conv2D(input, conv_pw1_ws, convp1_weights, name + "_convp1");
    Tensor norm1 = norm_layer(conv_pw1, conv_mu[0], conv_sigma[0], name + "_norm1");
    Tensor relu1 = relu6_layer(norm1, name + "_p1_relu6");
    fs.emplace_back(conv_pw1.f);
    //std::cout << "conv33 shape: " << conv3x3.shape << std::endl;
    
    // conv_depthwise
    const WeightShape conv_dw_ws = {input.shape[0]*k, weight_shape.w, weight_shape.h,
                                    weight_shape.pad, weight_shape.stride};
    Tensor conv_dw = conv2D(relu1, conv_dw_ws, convdw_weights, name + "_convdw");
    Tensor norm2 = norm_layer(conv_dw, conv_mu[1], conv_sigma[1], name + "_norm2");
    Tensor relu2 = relu6_layer(norm2, name + "_dw_relu6");
    fs.emplace_back(conv_dw.f);
    //std::cout << "conv31 shape: " << conv3x1.shape << std::endl;
    
    // conv_pointwise2
    const WeightShape conv_pw2_ws = {weight_shape.c, 1, 1, 0, 1};
    Tensor conv_pw2 = conv2D(relu2, conv_pw2_ws, convp2_weights, name + "_convp2");
    Tensor norm3 = norm_layer(conv_pw2, conv_mu[2], conv_sigma[2], name + "_norm3");
    fs.emplace_back(conv_pw2.f);
    //std::cout << "conv11 shape: " << conv1x1.shape << std::endl;

    Tensor output;
    if (weight_shape.stride == 1 && input.shape[0] == weight_shape.c) {
      // sum_layer
      //std::cout << "creating sum layer" << std::endl;
      output = sum_layer(conv_pw2, input, name + "_sum");
    } else {
      output = conv_pw2;
    }

    output.fs = fs;
    output.shape = compute_shape(input, weight_shape);
    return output;
  }

  Tensor bottleneck_block(const Tensor &input, const int expansion,
                          const WeightShape &weight_shape, const std::vector<Func> weights,
                          const std::string &name) {
    //std::cout << "making layer = " << name << std::endl;
    if (weights.size() != 9) { std::cout << "num weights = " << weights.size() << std::endl; }
    assert(weights.size() == 9);
    const Func &convp1_weights = weights[0];
    const Func &convdw_weights = weights[1];
    const Func &convp2_weights = weights[2]; 
    const Func conv_mu[3] = {weights[3], weights[4], weights[5]};
    const Func conv_sigma[3] = {weights[6], weights[7], weights[8]};

    return bottleneck_block(input, expansion, weight_shape,
                            convp1_weights, convdw_weights, convp2_weights, conv_mu, conv_sigma,
                            name);
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

HALIDE_REGISTER_GENERATOR(MobilenetGenerator, mobilenetv2)
