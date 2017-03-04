#include "opaque.hpp"
#include "image/typed_image.inl"
#include "base/atomic/atomic.hpp"
#include "base/memory/align.inl"
#include "base/math/vector.inl"

namespace rendering { namespace sensor {

Opaque::Opaque(int2 dimensions, float exposure) :
	Sensor(dimensions, exposure) {
	pixels_ = memory::allocate_aligned<float4>(dimensions.v[0] * dimensions.v[1]);
}

Opaque::~Opaque() {
	memory::free_aligned(pixels_);
}

void Opaque::clear() {
	const auto d = dimensions();
	for (int32_t i = 0, len = d.v[0] * d.v[1]; i < len; ++i) {
		pixels_[i] = float4(0.f);
	}
}

size_t Opaque::num_bytes() const {
	const auto d = dimensions();
	return d.v[0] * d.v[1] * sizeof(float4);
}

void Opaque::add_pixel(int2 pixel, float4_p color, float weight) {
	const auto d = dimensions();

	auto& value = pixels_[d.v[0] * pixel.v[1] + pixel.v[0]];
	value += float4(weight * color.xyz(), weight);
}

void Opaque::add_pixel_atomic(int2 pixel, float4_p color, float weight) {
	const auto d = dimensions();

	auto& value = pixels_[d.v[0] * pixel.v[1] + pixel.v[0]];
	atomic::add_assign(value.v[0], weight * color.v[0]);
	atomic::add_assign(value.v[1], weight * color.v[1]);
	atomic::add_assign(value.v[2], weight * color.v[2]);
	atomic::add_assign(value.v[3], weight);
}

void Opaque::resolve(int32_t begin, int32_t end, image::Float_4& target) const {
	float exposure_factor = exposure_factor_;

	for (int32_t i = begin; i < end; ++i) {
		auto& value = pixels_[i];

		float3 color = value.xyz() / value.v[3];

		target.at(i) = float4(exposure_factor * color, 1.f);
	}
}

}}
