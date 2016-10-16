#include "opaque.hpp"
#include "image/typed_image.inl"
#include "base/atomic/atomic.hpp"
#include "base/memory/align.inl"
#include "base/math/vector.inl"

namespace rendering { namespace sensor {

Opaque::Opaque(int2 dimensions, float exposure) :
	Sensor(dimensions, exposure) {
	pixels_ = memory::allocate_aligned<float4>(dimensions.x * dimensions.y);
}

Opaque::~Opaque() {
	memory::free_aligned(pixels_);
}

void Opaque::clear() {
	auto d = dimensions();
	for (int32_t i = 0, len = d.x * d.y; i < len; ++i) {
		pixels_[i] = float4(0.f, 0.f, 0.f, 0.f);
	}
}

size_t Opaque::num_bytes() const {
	auto d = dimensions();
	return d.x * d.y * sizeof(float4);
}

void Opaque::add_pixel(int2 pixel, const float4& color, float weight) {
	auto d = dimensions();

	auto& value = pixels_[d.x * pixel.y + pixel.x];
	value += float4(weight * color.xyz, weight);
}

void Opaque::add_pixel_atomic(int2 pixel, const float4& color, float weight) {
	auto d = dimensions();

	auto& value = pixels_[d.x * pixel.y + pixel.x];
	atomic::add_assign(value.x, weight * color.x);
	atomic::add_assign(value.y, weight * color.y);
	atomic::add_assign(value.z, weight * color.z);
	atomic::add_assign(value.w, weight);
}

void Opaque::resolve(int32_t begin, int32_t end, image::Image_float_4& target) const {
	float exposure_factor = exposure_factor_;

	for (int32_t i = begin; i < end; ++i) {
		auto& value = pixels_[i];

		float3 color = value.xyz / value.w;

		target.at(i) = float4(exposure_factor * color, 1.f);
	}
}

}}
