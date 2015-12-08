#pragma once

#include "typed_image.hpp"

namespace image {

template<typename T>
Typed_image<T>::Typed_image(const Image::Description& description) :
	Image(description),
	data_(new T[description.dimensions.x * description.dimensions.y]) {}

template<typename T>
Typed_image<T>::~Typed_image() {
	delete [] data_;
}

template<typename T>
void Typed_image<T>::resize(math::uint2 dimensions) {
	delete [] data_;

	description_.dimensions = dimensions;
}

template<typename T>
const T& Typed_image<T>::at(uint32_t index) const {
	return data_[index];
}

template<typename T>
T& Typed_image<T>::at(uint32_t index) {
	return data_[index];
}

template<typename T>
const T& Typed_image<T>::at(uint32_t x, uint32_t y) const {
	uint32_t i = y * description_.dimensions.x + x;
	return data_[i];
}

template<typename T>
T& Typed_image<T>::at(uint32_t x, uint32_t y) {
	uint32_t i = y * description_.dimensions.x + x;
	return data_[i];
}

}
