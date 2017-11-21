#include "volume_rasterizer.hpp"
#include "base/encoding/encoding.inl"
#include "core/image/typed_image.inl"
#include "base/spectrum/rgb.hpp"
#include "base/memory/align.hpp"
#include "base/math/vector3.inl"

namespace procedural::aurora {

Volume_rasterizer::Volume_rasterizer(const int3& dimensions) :
	dimensions_(dimensions),
	area_(dimensions[0] * dimensions[1]),
	samples_(memory::allocate_aligned<float3>(dimensions[0] * dimensions[1] * dimensions[2])) {}

Volume_rasterizer::~Volume_rasterizer() {
	memory::free_aligned(samples_);
}

void Volume_rasterizer::resolve(image::Byte3& target) const {
	const int32_t len = dimensions_[0] * dimensions_[1] * dimensions_[2];

	for (int32_t i = 0; i < len; ++i) {
		const float3 color = spectrum::linear_RGB_to_sRGB(samples_[i]);
		target.store(i, encoding::float_to_unorm(color));
	}
}

void Volume_rasterizer::set_brush(const float3& color) {
	brush_ = color;
}

void Volume_rasterizer::clear() {
	const int32_t len = dimensions_[0] * dimensions_[1] * dimensions_[2];

	for (int32_t i = 0; i < len; ++i) {
		samples_[i] = brush_;
	}
}

void Volume_rasterizer::splat(const float3& position, const float3& color) {
	const int3 c(position);

	if (c[0] < 0 || c[0] >= dimensions_[0]
	||  c[1] < 0 || c[1] >= dimensions_[1]
	||  c[2] < 0 || c[2] >= dimensions_[2]) {
		return;
	}

	const int32_t i = c[2] * area_ + c[1] * dimensions_[0] + c[0];

	samples_[i] = color;
}

float3* Volume_rasterizer::data() {
	return samples_;
}

}
