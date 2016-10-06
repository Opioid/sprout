#pragma once

#include "image.hpp"

namespace image {

template<typename T>
class Typed_image : public Image {

public:

	Typed_image(const Image::Description& description);
	~Typed_image();

	const T& at(int32_t index) const;
	T& at(int32_t index);

	T load(int32_t x, int32_t y) const;

	T load_element(int32_t x, int32_t y, int32_t element) const;

	const T& at(int32_t x, int32_t y) const;
	T& at(int32_t x, int32_t y);

	const T& at_element(int32_t x, int32_t y, int32_t element) const;
	T& at_element(int32_t x, int32_t y, int32_t element);

	T load(int32_t x, int32_t y, int32_t z) const;

	const T& at(int32_t x, int32_t y, int32_t z) const;
	T& at(int32_t x, int32_t y, int32_t z);

	const T& at_element(int32_t x, int32_t y, int32_t z, int32_t element) const;
	T& at_element(int32_t x, int32_t y, int32_t z, int32_t element);

	const T* data() const;

	virtual size_t num_bytes() const final override;

private:

	T* data_;
};

using Image_byte_1 = Typed_image<uint8_t>;
using Image_byte_2 = Typed_image<math::byte2>;
using Image_byte_3 = Typed_image<byte3>;
// using Image_float_3 = Typed_image<float3>;
using Image_float_3 = Typed_image<math::packed_float3>;
using Image_float_4 = Typed_image<float4>;

}
