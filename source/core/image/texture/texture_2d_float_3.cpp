#include "texture_2d_float_3.hpp"
#include "image/image.hpp"
#include "base/math/vector.inl"

namespace image { namespace texture {

Texture_2D_float_3::Texture_2D_float_3(std::shared_ptr<Image> image) :
	Texture_2D(image),
	data_(reinterpret_cast<const math::float3*>(image->data())) {}

float Texture_2D_float_3::at_1(uint32_t x, uint32_t y) const {
	uint32_t i = y * dimensions().x + x;

	return data_[i].x;
}

math::float2 Texture_2D_float_3::at_2(uint32_t x, uint32_t y) const {
	uint32_t i = y * dimensions().x + x;

	return data_[i].xy;
}

math::float3 Texture_2D_float_3::at_3(uint32_t x, uint32_t y) const {
	uint32_t i = y * dimensions().x + x;

	return data_[i];
}

math::float4 Texture_2D_float_3::at_4(uint32_t x, uint32_t y) const {
	uint32_t i = y * dimensions().x + x;

	return math::float4(data_[i], 1.f);
}

}}
