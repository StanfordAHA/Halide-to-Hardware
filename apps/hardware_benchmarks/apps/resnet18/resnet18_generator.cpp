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

// returns index of found value in array or -1 if not in array
int find_index(int value, std::vector<int> vec) {
    std::vector<int>::iterator it = std::find(vec.begin(), vec.end(), value);
    if (it == vec.end())
        return -1;
    return std::distance(vec.begin(), it);
}

std::ostream& operator<<(std::ostream& os, const std::vector<int>& vec);

class ResnetGenerator : public Halide::Generator<ResnetGenerator> {
public:
  Input<Buffer<float>> input{"input", 3};

  /** parameter values for scaling layers **/
  Input<Buffer<float>> conv1_gamma{"conv1_gamma", 1};
  Input<Buffer<float>[4]> br1_gamma  { "br1_gamma",  1 };
  Input<Buffer<float>[8]> br2a_gamma { "br2a_gamma", 1 };
  Input<Buffer<float>[8]> br2b_gamma { "br2b_gamma", 1 };

  Input<Buffer<float>> conv1_beta{"conv1_beta", 1};
  Input<Buffer<float>[4]> br1_beta  { "br1_beta",  1 };
  Input<Buffer<float>[8]> br2a_beta { "br2a_beta", 1 };
  Input<Buffer<float>[8]> br2b_beta { "br2b_beta", 1 };

  Input<Buffer<float>> conv1_mu{"conv1_mu", 1};
  Input<Buffer<float>[4]> br1_mu  { "br1_mu",  1 };
  Input<Buffer<float>[8]> br2a_mu { "br2a_mu", 1 };
  Input<Buffer<float>[8]> br2b_mu { "br2b_mu", 1 };

  Input<Buffer<float>> conv1_sig{"conv1_sig", 1};
  Input<Buffer<float>[4]> br1_sig  { "br1_sig",  1 };
  Input<Buffer<float>[8]> br2a_sig { "br2a_sig", 1 };
  Input<Buffer<float>[8]> br2b_sig { "br2b_sig", 1 };

  /** weights and biases for convolutions **/
  Input<Buffer<float>> conv1_weights{"conv1_weights", 4};
  Input<Buffer<float>[4]> br1_conv_weights  { "br1_conv_weights",  4 };
  Input<Buffer<float>[8]> br2a_conv_weights { "br2a_conv_weights", 4 };
  Input<Buffer<float>[8]> br2b_conv_weights { "br2b_conv_weights", 4 };

  Input<Buffer<float>> fc1000_weights{"fc1000_weights", 2};
  Input<Buffer<float>> fc1000_bias{"fc1000_bias", 1};
 
  // k_ic determines the number of input channels in a single xcel run
  GeneratorParam<int> k_ic{"k_ic", 4};    // default: 4
  
  // k_oc determines the number of output channels in a single xcel run
  GeneratorParam<int> k_oc{"k_oc", 3};    // default: 3
  
  Output<Buffer<float>> final_output{"final_output", 1};

  /** list out shapes of each layers weights **/
  // In channels inferred by input tensor shape
  // weight shapes: out channels, kernel_w, kernel_h, pad, stride.
  const WeightShape conv1_ws = {64, 7, 7, 3, 2};
  const WeightShape pool1_ws = {64, 3, 3, 1, 2};
  const WeightShape pool5_ws = {2048, 7, 7, 0, 1};
  const WeightShape fc1000_ws = {1000, 1, 1, 0, 1};  // 1x1 conv with 2048 input channels and 1000 output channels

  // res2a, res2b both have shame shapes
  const WeightShape res2x_ws = {64, 3, 3, 1, 1};
  const WeightShape res2a_br1_ws = {64, 1, 1, 0, 1};

  // res3x is same for most layers
  const WeightShape res3_ws  = {128, 3, 3, 1, 2};
  const WeightShape res3x_ws = {128, 3, 3, 1, 1};
  const WeightShape res3a_br1_ws = {128, 1, 1, 0, 2};

  const WeightShape res4_ws  = {256, 3, 3, 1, 2};
  const WeightShape res4x_ws = {256, 3, 3, 1, 1};
  const WeightShape res4a_br1_ws = {256, 1, 1, 0, 2};

  const WeightShape res5_ws  = {512, 3, 3, 1, 2};
  const WeightShape res5x_ws = {512, 3, 3, 1, 1};
  const WeightShape res5a_br1_ws = {512, 1, 1, 0, 2};

  const WeightShape br1_ws[4] = {res2a_br1_ws, res3a_br1_ws, res4a_br1_ws, res5a_br1_ws};
  const WeightShape br2a_ws[8] = {res2x_ws, res2x_ws, 
                                  res3_ws, res3x_ws, 
                                  res4_ws, res4x_ws, 
                                  res5_ws, res5x_ws};
  const WeightShape br2b_ws[8] = {res2x_ws, res2x_ws, 
                                  res3x_ws, res3x_ws, 
                                  res4x_ws, res4x_ws, 
                                  res5x_ws, res5x_ws};
  Var c, i, j;

  void generate() {

    // Algorithm
    /** Declare arrays of other functions and build the requested block **/
    Tensor br1_conv[4];
    Tensor br1_norm[4];
    Tensor br1_scale[4];

    Tensor br2a_conv[8];
    Tensor br2a_norm[8];
    Tensor br2a_scaled[8];
    Tensor br2a_relu[8];

    Tensor br2b_conv[8];
    Tensor br2b_norm[8];
    Tensor br2b_scaled[8];
    Tensor br2b_relu[8];

    Tensor br2c_conv[8];
    Tensor br2c_norm[8];
    Tensor br2c_scaled[8];

    Tensor resunit_sum[8];
    Tensor resunit_relu[8];

    Tensor pool5;
    Tensor fc1000;
    Tensor softmax;
    //Func hw_output("hw_output");

    // these tensors are different depending on the block and must be conditionally assigned.
    Tensor input_t;
    input_t.name = "input";
    std::vector<int> input_shape;
    Tensor br2a_input;
    Tensor resunit_sum_input;

    // used only for block_id == 0
    Tensor conv1, norm1, scaled1, relu1, pool1;

    std::vector<int> branch1_indices{0, 2, 4, 6};

    /** if block_id is 0 build the (stem) conv1 section **/
    int num_blocks = 8;
    for (int block_id = 0; block_id < num_blocks; ++block_id) {
      if (block_id == 0) {
        input_shape = {3, 224, 224};
        input_t.f = input;
        input_t.shape = input_shape;

        conv1 = conv2D(input_t, conv1_ws, conv1_weights, "conv1");
        norm1 = norm_layer(conv1, conv1_mu, conv1_sig, "norm1");
        scaled1 = scale_layer(norm1, conv1_gamma, conv1_beta, "scale1");
        relu1 = relu_layer(scaled1, "relu1");
        pool1 = max_pool_layer(relu1, pool1_ws, "pool1");

        br2a_input = pool1;
      } else {
        br2a_input = resunit_relu[block_id - 1];
      }

      // build dotted skip connection if this section has branch1
      int br1_i = find_index(block_id, branch1_indices);
      if (br1_i >= 0) {
        br1_conv[br1_i] = conv2D(br2a_input, br1_ws[br1_i], br1_conv_weights[br1_i], "br1_conv");
        br1_norm[br1_i] = norm_layer(br1_conv[br1_i], br1_mu[br1_i], br1_sig[br1_i], "br1_norm");
        br1_scale[br1_i] = scale_layer(br1_norm[br1_i], br1_gamma[br1_i], br1_beta[br1_i], "br1_scale");
        resunit_sum_input = br1_scale[br1_i];
      } else {
        resunit_sum_input = resunit_relu[block_id - 1];
      }

      // branch a
      auto weights = br2a_conv_weights[block_id];
      std::string bid = std::to_string(block_id);

      br2a_conv[block_id] = conv2D(br2a_input, br2a_ws[block_id], weights, "block" + bid + "_2a_conv");
      br2a_norm[block_id] = norm_layer(br2a_conv[block_id], br2a_mu[block_id], br2a_sig[block_id], "block" + bid + "_2a_norm");
      br2a_scaled[block_id] = scale_layer(br2a_norm[block_id], br2a_gamma[block_id], br2a_beta[block_id], "block" + bid + "_2a_scale");
      br2a_relu[block_id] = relu_layer(br2a_scaled[block_id], "2a_relu");

      // branch b
      weights = br2b_conv_weights[block_id];
      br2b_conv[block_id] = conv2D(br2a_relu[block_id], br2b_ws[block_id], weights, "block" + bid + "_2b_conv");
      br2b_norm[block_id] = norm_layer(br2b_conv[block_id], br2b_mu[block_id], br2b_sig[block_id], "block" + bid + "_2b_norm");
      br2b_scaled[block_id] = scale_layer(br2b_norm[block_id], br2b_gamma[block_id], br2b_beta[block_id], "block" + bid + "_2b_scale");

      // create residual unit
      resunit_sum[block_id] = sum_layer(resunit_sum_input, br2b_scaled[block_id], "block" + bid + "_res_sum");
      resunit_relu[block_id] = relu_layer(resunit_sum[block_id], "block" + bid + "_res_relu");

      // create final 3 layers
      if (block_id == num_blocks-1) {
        pool5 = avg_pool_layer(resunit_relu[block_id], pool5_ws, "pool5");
        fc1000 = fc_layer(pool5, fc1000_ws, fc1000_weights, fc1000_bias, "fc1000");
        final_output = softmax_layer(fc1000, 1000, "softmax");
      }
    }

    final_output.bound(c, 0, 1000);

    //final_output.bound(c, 0, 32);
    //final_output.bound(x, 0, 1280);
    //final_output.bound(y, 0, 720);

    // TODO: Actually schedule this.
    if (get_target().has_feature(Target::Clockwork)) {
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
        int imgsize = 224;
        auto gbsize = imgsize; // necessary until host can stream in tiles

        // start tiling after last conv, before final 3 layers
        auto hw_output = resunit_relu[7].f;
        hw_output.compute_root();
        //hw_output.unroll(w, k_oc);
        hw_output
          //.tile(i, j, x_host,y_host, xi,yi, gbsize,gbsize)
          .tile(i, j, x_host,y_host, xi,yi, 1280,720) // all as one tile until host can stream tiles
          .reorder(xi,yi,c, x_host,y_host)
          .hw_accelerate(xi, x_host);

        //fc1000.f.compute_at(hw_output, x_host);
        //pool5.f.compute_at(hw_output, x_host);

        for (int block_id = 0; block_id < 8; ++block_id) {
          br2b_conv[block_id].f.compute_at(hw_output, x_host);
          br2a_conv[block_id].f.compute_at(hw_output, x_host);

          int br1_i = find_index(block_id, branch1_indices);
          if (br1_i >= 0) {
            br1_conv[block_id].f.compute_at(hw_output, x_host);
          }
        }

        conv1.f.compute_at(hw_output, x_host);

        // stream inputs into the accelerator
        input.in().stream_to_accelerator();
      
        conv1_gamma.in().stream_to_accelerator();
        conv1_beta.in().stream_to_accelerator();
        conv1_mu.in().stream_to_accelerator();
        conv1_sig.in().stream_to_accelerator();
        conv1_weights.in().stream_to_accelerator();

        for (size_t i=0; i<8; ++i) {
          br2a_gamma[i].in().stream_to_accelerator();
          br2b_gamma[i].in().stream_to_accelerator();

          br2a_beta[i].in().stream_to_accelerator();
          br2b_beta[i].in().stream_to_accelerator();

          br2a_mu[i].in().stream_to_accelerator();
          br2b_mu[i].in().stream_to_accelerator();

          br2a_sig[i].in().stream_to_accelerator();
          br2b_sig[i].in().stream_to_accelerator();

          br2a_conv_weights[i].in().stream_to_accelerator();
          br2b_conv_weights[i].in().stream_to_accelerator();
        }

        for (size_t i=0; i<4; ++i) {
          br1_gamma[i].in().stream_to_accelerator();
          br1_beta[i].in().stream_to_accelerator();
          br1_mu[i].in().stream_to_accelerator();
          br1_sig[i].in().stream_to_accelerator();
          br1_conv_weights[i].in().stream_to_accelerator();
        }

        fc1000_weights.in().stream_to_accelerator();
        fc1000_bias.in().stream_to_accelerator();
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
    if (t1.shape != t2.shape) { std::cout << name << " does not match: " << t1.shape << " vs " << t2.shape << std::endl; }
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
    Tensor conv3x3 = conv2D(input, conv3x3_ws, convwh_weights, name + "_conv3x3");
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

HALIDE_REGISTER_GENERATOR(ResnetGenerator, resnet18)
