// MIT License
// Copyright (c) 2022 - 傅莘莘
// Source URL: https://github.com/zjhellofss/KuiperInfer
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

// Created by fss on 22-11-13.

#include "convolution.hpp"
#include <glog/logging.h>
#include "layer/abstract/layer_factory.hpp"
#include "runtime/runtime_ir.hpp"
#include "tick.hpp"
#include "utils/math/fmath.hpp"

namespace kuiper_infer {

void ConvolutionLayer::InitIm2ColWeight() {
  const uint32_t kernel_count = this->weights_.size();
  CHECK(kernel_count > 0) << "kernel count must greater than zero";
  const uint32_t kernel_h = this->weights_.at(0)->rows();
  const uint32_t kernel_w = this->weights_.at(0)->cols();
  const uint32_t kernel_c = this->weights_.at(0)->channels();
  const uint32_t row_len = kernel_h * kernel_w;
  CHECK(kernel_h > 0 && kernel_w > 0 && kernel_c > 0)
      << "The size of kernel matrix should be greater than zero";

  for (uint32_t k = 0; k < kernel_count; ++k) {
    const std::shared_ptr<Tensor<float>>& kernel = this->weights_.at(k);
    CHECK(kernel->rows() == kernel_h);
    CHECK(kernel->cols() == kernel_w);
    CHECK(kernel->channels() == kernel_c);
  }

  std::vector<arma::frowvec> kernel_matrix_arr(kernel_count);
  arma::frowvec kernel_matrix_c(row_len * kernel_c);
  for (uint32_t k = 0; k < kernel_count; ++k) {
    const std::shared_ptr<Tensor<float>>& kernel = this->weights_.at(k);
    for (uint32_t ic = 0; ic < kernel->channels(); ++ic) {
      memcpy(kernel_matrix_c.memptr() + row_len * ic, kernel->matrix_raw_ptr(ic),
             row_len * sizeof(float));
    }
    kernel_matrix_arr.at(k) = kernel_matrix_c;
  }

  if (!kernel_matrix_arr.empty()) {
    if (groups_ == 1) {
      CHECK(kernel_matrix_arr.size() == kernel_count / groups_)
          << "The number of kernel matrix and kernel_count_group do not match";
    } else {
      CHECK(kernel_matrix_arr.size() == kernel_count)
          << "The number of kernel matrix and kernel_count do not match";
    }
  }

  this->kernel_matrix_arr_ = std::move(kernel_matrix_arr);
}

void ConvolutionLayer::ComputeOutput(sftensor input, sftensor output_tensor, uint32_t kernel_h,
                                     uint32_t kernel_w, uint32_t kernel_count_group,
                                     uint32_t input_h, uint32_t input_w,
                                     uint32_t channels_per_group, uint32_t output_h,
                                     uint32_t output_w, uint32_t group) const {
  const arma::fmat& input_matrix =
      ConvIm2Col(input, kernel_h, kernel_w, input_h, input_w, channels_per_group, output_h,
                 output_w, group, kernel_h * kernel_w, output_h * output_w);
#pragma omp parallel for
  for (uint32_t k = 0; k < kernel_count_group; ++k) {
    ConvGemmBias(input_matrix, output_tensor, group, k, kernel_count_group, output_h, output_w);
  }
}

arma::fmat ConvolutionLayer::ConvIm2Col(sftensor input, uint32_t kernel_h, uint32_t kernel_w,
                                        uint32_t input_h, uint32_t input_w,
                                        uint32_t channels_per_group, uint32_t output_h,
                                        uint32_t output_w, uint32_t group, uint32_t row_len,
                                        uint32_t col_len) const {
  CHECK(input && !input->empty());
  const float padding_value = 0.f;
  arma::fmat input_matrix(channels_per_group * row_len, col_len);

#pragma omp parallel for
  for (uint32_t ic = 0; ic < channels_per_group; ++ic) {
    float* input_channel_ptr = input->matrix_raw_ptr(ic + group * channels_per_group);
    uint32_t current_col = 0;
    uint32_t channel_row = ic * row_len;
    for (uint32_t w = 0, iw = 0; w < output_w; ++w, iw += stride_w_) {
      for (uint32_t r = 0, ih = 0; r < output_h; ++r, ih += stride_h_) {
        float* input_matrix_ptr = input_matrix.colptr(current_col) + channel_row;
        current_col += 1;
        for (uint32_t kw = 0; kw < kernel_w * dilation_w_; kw += dilation_w_) {
          const uint32_t region_w = input_h * (iw + kw - padding_w_);
          for (uint32_t kh = 0; kh < kernel_h * dilation_h_; kh += dilation_h_) {
            if ((kh + ih >= padding_h_ && kw + iw >= padding_w_) &&
                (kh + ih < input_h + padding_h_ && kw + iw < input_w + padding_w_)) {
              float* region_ptr = input_channel_ptr + region_w + (ih + kh - padding_h_);
              *(input_matrix_ptr++) = *region_ptr;
            } else {
              *(input_matrix_ptr++) = padding_value;  // only support zero mode
            }
          }
        }
      }
    }
  }
  return input_matrix;
}

void ConvolutionLayer::ConvGemmBias(const arma::fmat& input_matrix, sftensor output_tensor,
                                    uint32_t group, uint32_t kernel_index,
                                    uint32_t kernel_count_group, uint32_t output_h,
                                    uint32_t output_w) const {
  CHECK(!input_matrix.empty());
  CHECK(output_tensor && !output_tensor->empty());

  kernel_index = kernel_index + group * kernel_count_group;
  const arma::frowvec& kernel = this->kernel_matrix_arr_.at(kernel_index);

  arma::fmat output(output_tensor->matrix_raw_ptr(kernel_index), output_h, output_w, false, true);
  output = kernel * input_matrix;

  return AddBias(output, kernel_index);
}

std::pair<uint32_t, uint32_t> ConvolutionLayer::ComputeOutputSize(const uint32_t input_h,
                                                                  const uint32_t input_w,
                                                                  const uint32_t kernel_h,
                                                                  const uint32_t kernel_w) const {
  uint32_t output_h = 0;
  uint32_t output_w = 0;

  CHECK_GT(kernel_h, 0);
  CHECK_GT(kernel_w, 0);

  output_h = (input_h + 2 * padding_h_ - dilation_h_ * (kernel_h - 1) - 1) / stride_h_ + 1;
  output_w = (input_w + 2 * padding_w_ - dilation_w_ * (kernel_w - 1) - 1) / stride_w_ + 1;
  return {output_h, output_w};
}

LayerRegistererWrapper kConvCreateInstance("nn.Conv2d", BaseConvolutionLayer::CreateInstance);

}  // namespace kuiper_infer