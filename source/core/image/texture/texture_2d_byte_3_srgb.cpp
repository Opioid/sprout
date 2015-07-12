#include "texture_2d_byte_3_srgb.hpp"
#include "texture_encoding.hpp"
#include "image/image.hpp"
#include "base/math/vector.inl"

namespace image { namespace texture {

Texture_2D_byte_3_sRGB::Texture_2D_byte_3_sRGB(std::shared_ptr<Image> image) :
	Texture_2D(image),
	data_(reinterpret_cast<const color::Color3c*>(image->data())) {}

float Texture_2D_byte_3_sRGB::at_1(uint32_t x, uint32_t y) const {
	uint32_t i = y * dimensions().x + x;

	return 0.f;
}

math::float2 Texture_2D_byte_3_sRGB::at_2(uint32_t x, uint32_t y) const {
	uint32_t i = y * dimensions().x + x;

	return math::float2::identity;
}

math::float3 Texture_2D_byte_3_sRGB::at_3(uint32_t x, uint32_t y) const {
	uint32_t i = y * dimensions().x + x;

	return math::float3(encoding::srgb_to_float(data_[i].x), encoding::srgb_to_float(data_[i].y), encoding::srgb_to_float(data_[i].z));
}

math::float4 Texture_2D_byte_3_sRGB::at_4(uint32_t x, uint32_t y) const {
	uint32_t i = y * dimensions().x + x;

	return math::float4::identity;
}

}}
