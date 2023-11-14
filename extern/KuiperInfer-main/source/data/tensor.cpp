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

// Created by fss on 22-11-12.

#include "data/tensor.hpp"
namespace kuiper_infer {

template <typename T>
Tensor<T>::Tensor(uint32_t channels, uint32_t rows, uint32_t cols) {
  data_ = arma::Cube<T>(rows, cols, channels);
  if (channels == 1 && rows == 1) {
    this->raw_shapes_ = std::vector<uint32_t>{cols};
  } else if (channels == 1) {
    this->raw_shapes_ = std::vector<uint32_t>{rows, cols};
  } else {
    this->raw_shapes_ = std::vector<uint32_t>{channels, rows, cols};
  }
}

template <typename T>
Tensor<T>::Tensor(uint32_t size) {
  data_ = arma::Cube<T>(1, size, 1);
  this->raw_shapes_ = std::vector<uint32_t>{size};
}

template <typename T>
Tensor<T>::Tensor(uint32_t rows, uint32_t cols) {
  data_ = arma::Cube<T>(rows, cols, 1);
  if (rows == 1) {
    this->raw_shapes_ = std::vector<uint32_t>{cols};
  } else {
    this->raw_shapes_ = std::vector<uint32_t>{rows, cols};
  }
}

template <typename T>
Tensor<T>::Tensor(const std::vector<uint32_t>& shapes) {
  CHECK(!shapes.empty() && shapes.size() <= 3);

  uint32_t remaining = 3 - shapes.size();
  std::vector<uint32_t> shapes_(3, 1);
  std::copy(shapes.begin(), shapes.end(), shapes_.begin() + remaining);

  uint32_t channels = shapes_.at(0);
  uint32_t rows = shapes_.at(1);
  uint32_t cols = shapes_.at(2);

  data_ = arma::Cube<T>(rows, cols, channels);
  if (channels == 1 && rows == 1) {
    this->raw_shapes_ = std::vector<uint32_t>{cols};
  } else if (channels == 1) {
    this->raw_shapes_ = std::vector<uint32_t>{rows, cols};
  } else {
    this->raw_shapes_ = std::vector<uint32_t>{channels, rows, cols};
  }
}

template <typename T>
uint32_t Tensor<T>::rows() const {
  CHECK(!this->data_.empty());
  return this->data_.n_rows;
}

template <typename T>
uint32_t Tensor<T>::cols() const {
  CHECK(!this->data_.empty());
  return this->data_.n_cols;
}

template <typename T>
uint32_t Tensor<T>::channels() const {
  CHECK(!this->data_.empty());
  return this->data_.n_slices;
}

template <typename T>
size_t Tensor<T>::size() const {
  CHECK(!this->data_.empty());
  return this->data_.size();
}

template <typename T>
void Tensor<T>::set_data(const arma::Cube<T>& data) {
  CHECK(data.n_rows == this->data_.n_rows) << data.n_rows << " != " << this->data_.n_rows;
  CHECK(data.n_cols == this->data_.n_cols) << data.n_cols << " != " << this->data_.n_cols;
  CHECK(data.n_slices == this->data_.n_slices) << data.n_slices << " != " << this->data_.n_slices;
  this->data_ = data;
}

template <typename T>
bool Tensor<T>::empty() const {
  return this->data_.empty();
}

template <typename T>
T Tensor<T>::index(uint32_t offset) const {
  CHECK(offset < this->data_.size()) << "Tensor index out of bound!";
  return this->data_.at(offset);
}

template <typename T>
T& Tensor<T>::index(uint32_t offset) {
  CHECK(offset < this->data_.size()) << "Tensor index out of bound!";
  return this->data_.at(offset);
}

template <typename T>
std::vector<uint32_t> Tensor<T>::shapes() const {
  CHECK(!this->data_.empty());
  return {this->channels(), this->rows(), this->cols()};
}

template <typename T>
arma::Cube<T>& Tensor<T>::data() {
  return this->data_;
}

template <typename T>
const arma::Cube<T>& Tensor<T>::data() const {
  return this->data_;
}

template <typename T>
arma::Mat<T>& Tensor<T>::slice(uint32_t channel) {
  CHECK_LT(channel, this->channels());
  return this->data_.slice(channel);
}

template <typename T>
const arma::Mat<T>& Tensor<T>::slice(uint32_t channel) const {
  CHECK_LT(channel, this->channels());
  return this->data_.slice(channel);
}

template <typename T>
T Tensor<T>::at(uint32_t channel, uint32_t row, uint32_t col) const {
  CHECK_LT(row, this->rows());
  CHECK_LT(col, this->cols());
  CHECK_LT(channel, this->channels());
  return this->data_.at(row, col, channel);
}

template <typename T>
T& Tensor<T>::at(uint32_t channel, uint32_t row, uint32_t col) {
  CHECK_LT(row, this->rows());
  CHECK_LT(col, this->cols());
  CHECK_LT(channel, this->channels());
  return this->data_.at(row, col, channel);
}

template <typename T>
void Tensor<T>::Padding(const std::vector<uint32_t>& pads, T padding_value) {
  CHECK(!this->data_.empty());
  CHECK_EQ(pads.size(), 4);
  uint32_t pad_rows1 = pads.at(0);  // up
  uint32_t pad_rows2 = pads.at(1);  // bottom
  uint32_t pad_cols1 = pads.at(2);  // left
  uint32_t pad_cols2 = pads.at(3);  // right

  arma::Cube<T> new_data(this->data_.n_rows + pad_rows1 + pad_rows2,
                         this->data_.n_cols + pad_cols1 + pad_cols2, this->data_.n_slices);
  new_data.fill(padding_value);

  new_data.subcube(pad_rows1, pad_cols1, 0, new_data.n_rows - pad_rows2 - 1,
                   new_data.n_cols - pad_cols2 - 1, new_data.n_slices - 1) = this->data_;
  this->data_ = std::move(new_data);
  this->raw_shapes_ = std::vector<uint32_t>{this->channels(), this->rows(), this->cols()};
}

template <typename T>
void Tensor<T>::Fill(T value) {
  CHECK(!this->data_.empty());
  this->data_.fill(value);
}

template <typename T>
void Tensor<T>::Fill(const std::vector<T>& values, bool row_major) {
  CHECK(!this->data_.empty());
  const uint32_t total_elems = this->data_.size();
  CHECK_EQ(values.size(), total_elems);
  if (row_major) {
    const uint32_t rows = this->rows();
    const uint32_t cols = this->cols();
    const uint32_t planes = rows * cols;
    const uint32_t channels = this->data_.n_slices;

    for (uint32_t i = 0; i < channels; ++i) {
      arma::Mat<T>& channel_data = this->data_.slice(i);
      arma::Mat<T> channel_data_t((T*)values.data() + i * planes, this->cols(), this->rows(), false,
                                  true);
      channel_data = channel_data_t.t();
    }
  } else {
    std::copy(values.begin(), values.end(), this->data_.memptr());
  }
}

template <typename T>
void Tensor<T>::Show() {
  for (uint32_t i = 0; i < this->channels(); ++i) {
    LOG(INFO) << "Channel: " << i;
    LOG(INFO) << "\n" << this->data_.slice(i);
  }
}

template <typename T>
void Tensor<T>::Flatten(bool row_major) {
  CHECK(!this->data_.empty());
  const uint32_t size = this->data_.size();
  this->Reshape({size}, row_major);
}

template <>
void Tensor<float>::RandN(float mean, float var) {
  CHECK(!this->data_.empty());
  std::random_device rd;
  std::mt19937 mt(rd());

  std::normal_distribution<float> dist(mean, var);
  for (size_t i = 0; i < this->size(); ++i) {
    this->index(i) = dist(mt);
  }
}

template <>
void Tensor<int32_t>::RandU(int32_t min, int32_t max) {
  CHECK(!this->data_.empty());
  std::random_device rd;
  std::mt19937 mt(rd());

  std::uniform_int_distribution<int32_t> dist(min, max);
  for (size_t i = 0; i < this->size(); ++i) {
    this->index(i) = dist(mt);
  }
}

template <>
void Tensor<std::uint8_t>::RandU(std::uint8_t min, std::uint8_t max) {
  CHECK(!this->data_.empty());
  std::random_device rd;
  std::mt19937 mt(rd());

#ifdef _MSC_VER
  std::uniform_int_distribution<int32_t> dist(min, max);
  uint8_t max_value = std::numeric_limits<uint8_t>::max();
  for (uint32_t i = 0; i < this->size(); ++i) {
    this->index(i) = dist(mt) % max_value;
  }
#else
  std::uniform_int_distribution<std::uint8_t> dist(min, max);
  for (size_t i = 0; i < this->size(); ++i) {
    this->index(i) = dist(mt);
  }
#endif
}

template <>
void Tensor<float>::RandU(float min, float max) {
  CHECK(!this->data_.empty());
  CHECK(max >= min);
  std::random_device rd;
  std::mt19937 mt(rd());
  std::uniform_real_distribution<float> dist(min, max);
  for (size_t i = 0; i < this->size(); ++i) {
    this->index(i) = dist(mt);
  }
}

template <typename T>
void Tensor<T>::Ones() {
  CHECK(!this->data_.empty());
  this->Fill(T{1});
}

template <typename T>
void Tensor<T>::Transform(const std::function<T(T)>& filter) {
  CHECK(!this->data_.empty());
  this->data_.transform(filter);
}

template <typename T>
const std::vector<uint32_t>& Tensor<T>::raw_shapes() const {
  CHECK(!this->raw_shapes_.empty());
  CHECK_LE(this->raw_shapes_.size(), 3);
  CHECK_GE(this->raw_shapes_.size(), 1);
  return this->raw_shapes_;
}

template <typename T>
void Tensor<T>::Reshape(const std::vector<uint32_t>& shapes, bool row_major) {
  CHECK(!this->data_.empty());
  CHECK(!shapes.empty());
  const size_t origin_size = this->size();
  const size_t current_size =
      std::accumulate(shapes.begin(), shapes.end(), 1, std::multiplies<size_t>());
  CHECK(shapes.size() <= 3);
  CHECK(current_size == origin_size);

  if (!row_major) {
    if (shapes.size() == 3) {
      this->data_.reshape(shapes.at(1), shapes.at(2), shapes.at(0));
      this->raw_shapes_ = {shapes.at(0), shapes.at(1), shapes.at(2)};
    } else if (shapes.size() == 2) {
      this->data_.reshape(shapes.at(0), shapes.at(1), 1);
      this->raw_shapes_ = {shapes.at(0), shapes.at(1)};
    } else {
      this->data_.reshape(1, shapes.at(0), 1);
      this->raw_shapes_ = {shapes.at(0)};
    }
  } else {
    if (shapes.size() == 3) {
      this->Review({shapes.at(0), shapes.at(1), shapes.at(2)});
      this->raw_shapes_ = {shapes.at(0), shapes.at(1), shapes.at(2)};
    } else if (shapes.size() == 2) {
      this->Review({1, shapes.at(0), shapes.at(1)});
      this->raw_shapes_ = {shapes.at(0), shapes.at(1)};
    } else {
      this->Review({1, 1, shapes.at(0)});
      this->raw_shapes_ = {shapes.at(0)};
    }
  }
}

template <typename T>
T* Tensor<T>::raw_ptr() {
  CHECK(!this->data_.empty());
  return this->data_.memptr();
}

template <typename T>
T* Tensor<T>::raw_ptr(size_t offset) {
  const size_t size = this->size();
  CHECK(!this->data_.empty());
  CHECK_LT(offset, size);
  return this->data_.memptr() + offset;
}

template <typename T>
std::vector<T> Tensor<T>::values(bool row_major) {
  CHECK_EQ(this->data_.empty(), false);
  std::vector<T> values(this->data_.size());

  if (!row_major) {
    std::copy(this->data_.mem, this->data_.mem + this->data_.size(), values.begin());
  } else {
    uint32_t index = 0;
    for (uint32_t c = 0; c < this->data_.n_slices; ++c) {
      const arma::Mat<T>& channel = this->data_.slice(c).t();
      std::copy(channel.begin(), channel.end(), values.begin() + index);
      index += channel.size();
    }
    CHECK_EQ(index, values.size());
  }
  return values;
}

template <typename T>
T* Tensor<T>::matrix_raw_ptr(uint32_t index) {
  CHECK_LT(index, this->channels());
  size_t offset = index * this->rows() * this->cols();
  CHECK_LE(offset, this->size());
  T* mem_ptr = this->raw_ptr(offset);
  return mem_ptr;
}

template <typename T>
void Tensor<T>::set_data(arma::Cube<T>&& data) {
  CHECK(data.n_rows == this->data_.n_rows) << data.n_rows << " != " << this->data_.n_rows;
  CHECK(data.n_cols == this->data_.n_cols) << data.n_cols << " != " << this->data_.n_cols;
  CHECK(data.n_slices == this->data_.n_slices) << data.n_slices << " != " << this->data_.n_slices;
  this->data_ = std::move(data);
}

template <typename T>
void Tensor<T>::Review(const std::vector<uint32_t>& shapes) {
  CHECK(!this->data_.empty());
  CHECK_EQ(shapes.size(), 3);
  const uint32_t target_channels = shapes.at(0);
  const uint32_t target_rows = shapes.at(1);
  const uint32_t target_cols = shapes.at(2);

  CHECK_EQ(this->data_.size(), target_channels * target_cols * target_rows);
  arma::Cube<T> new_data(target_rows, target_cols, target_channels);
  const uint32_t plane_size = target_rows * target_cols;
#pragma omp parallel for
  for (uint32_t channel = 0; channel < this->data_.n_slices; ++channel) {
    const arma::Mat<T>& channel_data = this->data_.slice(channel);
    const uint32_t plane_start = channel * data_.n_rows * data_.n_cols;
    for (uint32_t src_col = 0; src_col < this->data_.n_cols; ++src_col) {
      const T* col_ptr = channel_data.colptr(src_col);
      for (uint32_t src_row = 0; src_row < this->data_.n_rows; ++src_row) {
        const uint32_t pos_index = plane_start + src_row * data_.n_cols + src_col;
        const uint32_t dest_channel = pos_index / plane_size;
        const uint32_t dest_row = (pos_index - dest_channel * plane_size) / target_cols;
        const uint32_t dest_col = (pos_index - dest_channel * plane_size - dest_row * target_cols);
        new_data.at(dest_row, dest_col, dest_channel) = *(col_ptr + src_row);
      }
    }
  }
  this->data_ = std::move(new_data);
}

template class Tensor<float>;
template class Tensor<int32_t>;
template class Tensor<uint8_t>;
}  // namespace kuiper_infer
