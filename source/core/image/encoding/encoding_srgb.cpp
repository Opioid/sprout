#include "encoding_srgb.hpp"
#include "image/typed_image.inl"
#include "base/spectrum/rgb.inl"
#include "base/math/vector.inl"

namespace image { namespace encoding {

Srgb::Srgb(math::int2 dimensions) : rgb_(new math::byte3[dimensions.x * dimensions.y]) {}

Srgb::~Srgb() {
	delete [] rgb_;
}

const math::byte3* Srgb::data() const {
	return rgb_;
}

void Srgb::to_sRGB(const image::Image_float_4& image, int32_t begin, int32_t end) {
	for (int32_t i = begin; i < end; ++i) {
		math::float3 color = image.at(i).xyz;
		color = spectrum::linear_to_sRGB(color);
		rgb_[i] = spectrum::to_byte(color);
	}
}

void Srgb::to_byte(const image::Image_float_4& image, int32_t begin, int32_t end) {
	for (int32_t i = begin; i < end; ++i) {
		math::float3 color = image.at(i).xyz;
		rgb_[i] = spectrum::to_byte(color);
	}
}

Srgb_alpha::Srgb_alpha(math::int2 dimensions) : rgba_(new math::byte4[dimensions.x * dimensions.y])
{}

Srgb_alpha::~Srgb_alpha() {
	delete [] rgba_;
}

const math::byte4* Srgb_alpha::data() const {
	return rgba_;
}

void Srgb_alpha::to_sRGB(const image::Image_float_4& image, int32_t begin, int32_t end) {
	for (int32_t i = begin; i < end; ++i) {
		math::float4 color = image.at(i);
		color = spectrum::linear_to_sRGB(color);
		rgba_[i] = spectrum::to_byte(color);
	}
}

void Srgb_alpha::to_byte(const image::Image_float_4& image, int32_t begin, int32_t end) {
	for (int32_t i = begin; i < end; ++i) {
		math::float4 color = image.at(i);
		rgba_[i] = spectrum::to_byte(color);
	}
}

}}
