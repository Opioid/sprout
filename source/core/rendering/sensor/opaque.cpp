#include "opaque.hpp"
#include "image/typed_image.inl"
#include "tonemapping/tonemapper.hpp"
#include "base/atomic/atomic.hpp"
#include "base/math/vector.inl"

namespace rendering { namespace sensor {

Opaque::Opaque(math::int2 dimensions, float exposure, std::unique_ptr<tonemapping::Tonemapper> tonemapper) :
	Sensor(dimensions, exposure, std::move(tonemapper)),
	pixels_(new Pixel[dimensions.x * dimensions.y]) {}

Opaque::~Opaque() {
	delete [] pixels_;
}

void Opaque::clear() {
	auto d = dimensions();
	for (int32_t i = 0, len = d.x * d.y; i < len; ++i) {
		pixels_[i].color = math::float3(0.f, 0.f, 0.f);
		pixels_[i].weight_sum = 0.f;
	}
}

void Opaque::add_pixel(math::int2 pixel, const math::float4& color, float weight) {
	auto d = dimensions();

	auto& value = pixels_[d.x * pixel.y + pixel.x];
	value.color += weight * color.xyz();
	value.weight_sum += weight;
}

void Opaque::add_pixel_atomic(math::int2 pixel, const math::float4& color, float weight) {
	auto d = dimensions();

	auto& value = pixels_[d.x * pixel.y + pixel.x];
	atomic::add_assign(value.color.x, weight * color.x);
	atomic::add_assign(value.color.y, weight * color.y);
	atomic::add_assign(value.color.z, weight * color.z);
	atomic::add_assign(value.weight_sum, weight);
}

void Opaque::resolve(int32_t begin, int32_t end) {
	for (int32_t i = begin; i < end; ++i) {
		auto& value = pixels_[i];

		math::float3 color = value.color / value.weight_sum;

		math::float3 exposed = expose(color, exposure_);

		math::float3 tonemapped = tonemapper_->tonemap(exposed);

		image_.at(i) = math::float4(tonemapped, 1.f);
	}
}

}}
