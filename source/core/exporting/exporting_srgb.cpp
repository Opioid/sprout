#include "exporting_srgb.hpp"
#include "image/typed_image.inl"
#include "base/color/color.inl"
#include "base/math/vector.inl"
#include "base/thread/thread_pool.hpp"

namespace exporting {

Srgb::Srgb(const math::uint2& dimensions) :rgba_(new color::Color4c[dimensions.x * dimensions.y]) {}

Srgb::~Srgb() {
	delete [] rgba_;
}

void Srgb::to_sRGB(const image::Image_float_4& image, uint32_t begin, uint32_t end) {
	for (uint32_t i = begin; i < end; ++i) {
		math::float4 color = image.at(i);
		color.xyz = color::linear_to_sRGB(color.xyz);
		rgba_[i] = color::to_byte(color);
	}
}

}
