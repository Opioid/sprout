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
const T& Typed_image<T>::at(uint32_t index) const {
	return data_[index];
}

template<typename T>
void Typed_image<T>::set(uint32_t index, const T& value) {
	data_[index] = value;
}

template<typename T>
const void* Typed_image<T>::data() const {
	return static_cast<const void*>(data_);
}

}
