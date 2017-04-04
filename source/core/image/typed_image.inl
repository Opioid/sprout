#pragma once

#include "typed_image.hpp"
#include "base/memory/align.hpp"

namespace image {

template<typename T>
Typed_image<T>::Typed_image() :
	data_(nullptr) {}

template<typename T>
Typed_image<T>::Typed_image(const Image::Description& description) :
	Image(description),
	data_(memory::allocate_aligned<T>(description.num_pixels())) {}

template<typename T>
Typed_image<T>::~Typed_image() {
	memory::free_aligned(data_);
}

template<typename T>
Typed_image<T> Typed_image<T>::clone() const {
	return Typed_image<T>(description_);
}

template<typename T>
void Typed_image<T>::resize(const Image::Description& description) {
	memory::free_aligned(data_);

	Image::resize(description);

	data_ = memory::allocate_aligned<T>(description.num_pixels());
}

template<typename T>
void Typed_image<T>::clear(T v) {
	for (int32_t i = 0; i < volume_; ++i) {
		data_[i] = v;
	}
}

template<typename T>
T Typed_image<T>::load(int32_t index) const {
	return data_[index];
}

template<typename T>
void Typed_image<T>::store(int32_t index, T v) {
	data_[index] = v;
}

template<typename T>
const T& Typed_image<T>::at(int32_t index) const {
	return data_[index];
}

template<typename T>
T& Typed_image<T>::at(int32_t index) {
	return data_[index];
}

template<typename T>
T Typed_image<T>::load(int32_t x, int32_t y) const {
	int32_t i = y * description_.dimensions[0] + x;
	return data_[i];
}

template<typename T>
void Typed_image<T>::store(int32_t x, int32_t y, T v) {
	int32_t i = y * description_.dimensions[0] + x;
	data_[i] = v;
}

template<typename T>
T Typed_image<T>::load_element(int32_t x, int32_t y, int32_t element) const {
	int32_t i = element * area_ + y * description_.dimensions[0] + x;
	return data_[i];
}

template<typename T>
const T& Typed_image<T>::at(int32_t x, int32_t y) const {
	int32_t i = y * description_.dimensions[0] + x;
	return data_[i];
}

template<typename T>
T& Typed_image<T>::at(int32_t x, int32_t y) {
	int32_t i = y * description_.dimensions[0] + x;
	return data_[i];
}

template<typename T>
const T& Typed_image<T>::at_element(int32_t x, int32_t y, int32_t element) const {
	int32_t i = element * area_ + y * description_.dimensions[0] + x;
	return data_[i];
}

template<typename T>
T& Typed_image<T>::at_element(int32_t x, int32_t y, int32_t element) {
	int32_t i = element * area_ + y * description_.dimensions[0] + x;
	return data_[i];
}

template<typename T>
T Typed_image<T>::load(int32_t x, int32_t y, int32_t z) const {
	int32_t i = z * area_ + y * description_.dimensions[0] + x;
	return data_[i];
}

template<typename T>
const T& Typed_image<T>::at(int32_t x, int32_t y, int32_t z) const {
	int32_t i = z * area_ + y * description_.dimensions[0] + x;
	return data_[i];
}

template<typename T>
T& Typed_image<T>::at(int32_t x, int32_t y, int32_t z) {
	int32_t i = z * area_ + y * description_.dimensions[0] + x;
	return data_[i];
}

template<typename T>
const T& Typed_image<T>::at_element(int32_t x, int32_t y, int32_t z, int32_t element) const {
	int32_t i = element * volume_ + z * area_ + y * description_.dimensions[0] + x;
	return data_[i];
}

template<typename T>
T& Typed_image<T>::at_element(int32_t x, int32_t y, int32_t z, int32_t element) {
	int32_t i = element * volume_ + z * area_ + y * description_.dimensions[0] + x;
	return data_[i];
}

template<typename T>
void Typed_image<T>::gather(const int4& xy_xy1, T c[4]) const {
	const int32_t width = description_.dimensions[0];

	const int32_t y0 = width * xy_xy1[1];

	c[0] = data_[y0 + xy_xy1[0]];
	c[1] = data_[y0 + xy_xy1[2]];

	const int32_t y1 = width * xy_xy1[3];

	c[2] = data_[y1 + xy_xy1[0]];
	c[3] = data_[y1 + xy_xy1[2]];
}

template<typename T>
void Typed_image<T>::square_transpose() {
	const int32_t n = description_.dimensions[0];
	for (int32_t y = 0, height = n - 2; y < height; ++y) {
		for (int32_t x = y + 1, width = n -1; x < width; ++x) {
			int32_t a = y * n + x;
			int32_t b = x * n + y;
			std::swap(data_[a], data_[b]);
		}
	}
}

template<typename T>
T* Typed_image<T>::data() const {
	return data_;
}

template<typename T>
size_t Typed_image<T>::num_bytes() const {
	return sizeof(*this) +
			description_.dimensions[0] *
			description_.dimensions[1] *
			description_.dimensions[2] *
			description_.num_elements * sizeof(T);
}

}
