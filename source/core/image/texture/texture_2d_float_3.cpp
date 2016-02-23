#include "texture_2d_float_3.hpp"
#include "image/typed_image.inl"
#include "base/math/vector.inl"

namespace image { namespace texture {

Texture_2D_float_3::Texture_2D_float_3(std::shared_ptr<Image> image) :
	Texture_2D(image),
	image_(*reinterpret_cast<const Image_float_3*>(image.get())) {}

float Texture_2D_float_3::at_1(int32_t x, int32_t y) const {
	return image_.load(x, y).x;
}

math::float2 Texture_2D_float_3::at_2(int32_t x, int32_t y) const {
	return image_.load(x, y).xy();
}

math::vec3 Texture_2D_float_3::at_3(int32_t x, int32_t y) const {
	return image_.load(x, y);
}

math::float4 Texture_2D_float_3::at_4(int32_t x, int32_t y) const {
	return math::float4(image_.load(x, y), 1.f);
}

float Texture_2D_float_3::at_1(int32_t x, int32_t y, int32_t element) const {
	return image_.at(x, y, element).x;
}

math::float2 Texture_2D_float_3::at_2(int32_t x, int32_t y, int32_t element) const {
	return image_.at(x, y, element).xy();
}

math::vec3 Texture_2D_float_3::at_3(int32_t x, int32_t y, int32_t element) const {
	return image_.at(x, y, element);
}

math::float4 Texture_2D_float_3::at_4(int32_t x, int32_t y, int32_t element) const {
	return math::float4(image_.at(x, y, element), 1.f);
}

}}
