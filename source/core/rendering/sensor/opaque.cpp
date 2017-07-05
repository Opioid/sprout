#include "opaque.hpp"
#include "image/typed_image.inl"
#include "base/atomic/atomic.hpp"
#include "base/memory/align.hpp"
#include "base/math/vector4.inl"

namespace rendering { namespace sensor {

Opaque::Opaque(int2 dimensions, float exposure) :
	Sensor(dimensions, exposure) {
	pixels_ = memory::allocate_aligned<float4>(dimensions[0] * dimensions[1]);
}

Opaque::~Opaque() {
	memory::free_aligned(pixels_);
}

void Opaque::clear() {
	const auto d = dimensions();
	for (int32_t i = 0, len = d[0] * d[1]; i < len; ++i) {
		pixels_[i] = float4(0.f);
	}
}

size_t Opaque::num_bytes() const {
	const auto d = dimensions();
	return d[0] * d[1] * sizeof(float4);
}

void Opaque::add_pixel(int2 pixel, const float4& color, float weight) {
	const auto d = dimensions();

	auto& value = pixels_[d[0] * pixel[1] + pixel[0]];
	value += float4(weight * color.xyz(), weight);
}

void Opaque::add_pixel_atomic(int2 pixel, const float4& color, float weight) {
	const auto d = dimensions();

	auto& value = pixels_[d[0] * pixel[1] + pixel[0]];
	atomic::add_assign(value[0], weight * color[0]);
	atomic::add_assign(value[1], weight * color[1]);
	atomic::add_assign(value[2], weight * color[2]);
	atomic::add_assign(value[3], weight);
}

void Opaque::resolve(int32_t begin, int32_t end, image::Float4& target) const {
	const float exposure_factor = exposure_factor_;

	for (int32_t i = begin; i < end; ++i) {
		auto& value = pixels_[i];

		const float3 color = value.xyz() / value[3];

		target.at(i) = float4(exposure_factor * color, 1.f);
	}
}

}}
