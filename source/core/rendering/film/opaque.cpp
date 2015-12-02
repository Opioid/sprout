#include "opaque.hpp"
#include "image/typed_image.inl"
#include "tonemapping/tonemapper.hpp"
#include "base/atomic/atomic.hpp"
#include "base/math/vector.inl"

namespace rendering { namespace film {

Opaque::Opaque(math::uint2 dimensions, float exposure, std::unique_ptr<tonemapping::Tonemapper> tonemapper) :
	Film(dimensions, exposure, std::move(tonemapper)),
	pixels_(new Pixel[dimensions.x * dimensions.y]) {}

Opaque::~Opaque() {
	delete [] pixels_;
}

void Opaque::clear() {
	auto d = dimensions();
	for (uint32_t i = 0, len = d.x * d.y; i < len; ++i) {
		pixels_[i].color = math::float3(0.f, 0.f, 0.f);
		pixels_[i].weight_sum = 0.f;
	}
}

void Opaque::add_pixel(uint32_t x, uint32_t y, const math::float4& color, float weight) {
	auto d = dimensions();
	if (x >= d.x || y >= d.y) {
		return;
	}

	auto& pixel = pixels_[d.x * y + x];
	pixel.color += weight * color.xyz();
	pixel.weight_sum += weight;
}

void Opaque::add_pixel_atomic(uint32_t x, uint32_t y, const math::float4& color, float weight) {
	auto d = dimensions();
	if (x >= d.x || y >= d.y) {
		return;
	}

	auto& pixel = pixels_[d.x * y + x];
	atomic::add_assign(pixel.color.x, weight * color.x);
	atomic::add_assign(pixel.color.y, weight * color.y);
	atomic::add_assign(pixel.color.z, weight * color.z);
	atomic::add_assign(pixel.weight_sum, weight);
}

void Opaque::resolve(uint32_t begin, uint32_t end) {
	for (uint32_t i = begin; i < end; ++i) {
		auto& pixel = pixels_[i];

		math::float3 color = pixel.color / pixel.weight_sum;

		math::float3 exposed = expose(color, exposure_);

		math::float3 tonemapped = tonemapper_->tonemap(exposed);

		image_.at(i) = math::float4(tonemapped, 1.f);
	}
}

}}
