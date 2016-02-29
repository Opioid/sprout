#include "opaque.hpp"
#include "image/typed_image.inl"
#include "tonemapping/tonemapper.hpp"
#include "base/atomic/atomic.hpp"
#include "base/memory/align.inl"
#include "base/math/vector.inl"

namespace rendering { namespace sensor {

Opaque::Opaque(math::int2 dimensions, const tonemapping::Tonemapper* tonemapper) :
	Sensor(dimensions, tonemapper) {
	pixels_ = memory::allocate_aligned<math::float4>(dimensions.x * dimensions.y);
}

Opaque::~Opaque() {
	memory::free_aligned(pixels_);
}

void Opaque::clear() {
	auto d = dimensions();
	for (int32_t i = 0, len = d.x * d.y; i < len; ++i) {
		pixels_[i] = math::float4(0.f, 0.f, 0.f, 0.f);
	}
}

void Opaque::add_pixel(math::int2 pixel, const math::float4& color, float weight) {
	auto d = dimensions();

	auto& value = pixels_[d.x * pixel.y + pixel.x];
	value += math::float4(weight * color.xyz, weight);
}

void Opaque::add_pixel_atomic(math::int2 pixel, const math::float4& color, float weight) {
	auto d = dimensions();

	auto& value = pixels_[d.x * pixel.y + pixel.x];
	atomic::add_assign(value.x, weight * color.x);
	atomic::add_assign(value.y, weight * color.y);
	atomic::add_assign(value.z, weight * color.z);
	atomic::add_assign(value.w, weight);
}

void Opaque::resolve(int32_t begin, int32_t end, image::Image_float_4& target) const {
	for (int32_t i = begin; i < end; ++i) {
		auto& value = pixels_[i];

		math::vec3 color = value.xyz / value.w;

		math::vec3 tonemapped = tonemapper_->tonemap(color);

		target.at(i) = math::float4(tonemapped, 1.f);
	}
}

}}
