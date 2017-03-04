#include "transparent.hpp"
#include "image/typed_image.inl"
#include "base/atomic/atomic.hpp"
#include "base/math/vector.inl"

namespace rendering { namespace sensor {

Transparent::Transparent(int2 dimensions, float exposure) :
	Sensor(dimensions, exposure),
	pixels_(new Pixel[dimensions.v[0] * dimensions.v[1]]) {}

Transparent::~Transparent() {
	delete[] pixels_;
}

void Transparent::clear() {
	const auto d = dimensions();
	for (int32_t i = 0, len = d.v[0] * d.v[1]; i < len; ++i) {
		pixels_[i].color = float4(0.f);
		pixels_[i].weight_sum = 0.f;
	}
}

size_t Transparent::num_bytes() const {
	const auto d = dimensions();
	return d.v[0] * d.v[1] * sizeof(Pixel);
}

void Transparent::add_pixel(int2 pixel, float4_p color, float weight) {
	const auto d = dimensions();

	auto& value = pixels_[d.v[0] * pixel.v[1] + pixel.v[0]];
	value.color += weight * color;
	value.weight_sum += weight;
}

void Transparent::add_pixel_atomic(int2 pixel, float4_p color, float weight) {
	const auto d = dimensions();

	auto& value = pixels_[d.v[0] * pixel.v[1] + pixel.v[0]];
	atomic::add_assign(value.color.v[0], weight * color.v[0]);
	atomic::add_assign(value.color.v[1], weight * color.v[1]);
	atomic::add_assign(value.color.v[2], weight * color.v[2]);
	atomic::add_assign(value.color.v[3], weight * color.v[3]);
	atomic::add_assign(value.weight_sum, weight);
}

void Transparent::resolve(int32_t begin, int32_t end, image::Float_4& target) const {
	float exposure_factor = exposure_factor_;

	for (int32_t i = begin; i < end; ++i) {
		auto& value = pixels_[i];

		float4 color = value.color / value.weight_sum;

		target.at(i) = float4(exposure_factor * color.xyz(), std::min(color.v[3], 1.f));
	}
}

}}
