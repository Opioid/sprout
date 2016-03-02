#pragma once

#include "image.hpp"

namespace image {

template<typename T>
class Typed_image : public Image {
public:

	Typed_image(const Image::Description& description);
	~Typed_image();

	T load(int32_t x, int32_t y) const;

	const T& at(int32_t index) const;
	T& at(int32_t index);

	const T& at(int32_t x, int32_t y) const;
	T& at(int32_t x, int32_t y);

	const T& at(int32_t x, int32_t y, int32_t element) const;
	T& at(int32_t x, int32_t y, int32_t element);

private:

	T* data_;
};

typedef Typed_image<uint8_t>	  Image_byte_1;
typedef Typed_image<math::byte2>  Image_byte_2;
typedef Typed_image<math::byte3>  Image_byte_3;
typedef Typed_image<math::float3> Image_float_3;
typedef Typed_image<math::float4> Image_float_4;

}
