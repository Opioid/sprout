#include "texture_2d_float_3.hpp"
#include "image/typed_image.inl"
#include "base/math/vector.inl"

namespace image { namespace texture {

Texture_2D_float_3::Texture_2D_float_3(std::shared_ptr<Image> image) :
	Texture_2D(image),
	image_(*reinterpret_cast<const Image_float_3*>(image.get())) {}

float Texture_2D_float_3::at_1(uint32_t x, uint32_t y) const {
	return image_.at(x, y).x;
}

math::float2 Texture_2D_float_3::at_2(uint32_t x, uint32_t y) const {
	return image_.at(x, y).xy;
}

math::float3 Texture_2D_float_3::at_3(uint32_t x, uint32_t y) const {
	return image_.at(x, y);
}

math::float4 Texture_2D_float_3::at_4(uint32_t x, uint32_t y) const {
	return math::float4(image_.at(x, y), 1.f);
}

}}
