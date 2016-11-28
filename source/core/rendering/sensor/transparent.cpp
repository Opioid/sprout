#include "transparent.hpp"
#include "image/typed_image.inl"
#include "base/atomic/atomic.hpp"
#include "base/math/vector.inl"

namespace rendering { namespace sensor {

Transparent::Transparent(int2 dimensions, float exposure) :
	Sensor(dimensions, exposure),
	pixels_(new Pixel[dimensions.x * dimensions.y]) {}

Transparent::~Transparent() {
	delete [] pixels_;
}

void Transparent::clear() {
	auto d = dimensions();
	for (int32_t i = 0, len = d.x * d.y; i < len; ++i) {
		pixels_[i].color = float4(0.f);
		pixels_[i].weight_sum = 0.f;
	}
}

size_t Transparent::num_bytes() const {
	auto d = dimensions();
	return d.x * d.y * sizeof(Pixel);
}

void Transparent::add_pixel(int2 pixel, float4_p color, float weight) {
	auto d = dimensions();

	auto& value = pixels_[d.x * pixel.y + pixel.x];
	value.color += weight * color;
	value.weight_sum += weight;
}

void Transparent::add_pixel_atomic(int2 pixel, float4_p color, float weight) {
	auto d = dimensions();

	auto& value = pixels_[d.x * pixel.y + pixel.x];
	atomic::add_assign(value.color.x, weight * color.x);
	atomic::add_assign(value.color.y, weight * color.y);
	atomic::add_assign(value.color.z, weight * color.z);
	atomic::add_assign(value.color.w, weight * color.w);
	atomic::add_assign(value.weight_sum, weight);
}

void Transparent::resolve(int32_t begin, int32_t end, image::Float_4& target) const {
	float exposure_factor = exposure_factor_;

	for (int32_t i = begin; i < end; ++i) {
		auto& value = pixels_[i];

		float4 color = value.color / value.weight_sum;

		target.at(i) = float4(exposure_factor * color.xyz, std::min(color.w, 1.f));
	}
}

}}
