#include "film.hpp"
#include "image/typed_image.inl"
#include "tonemapping/tonemapper.hpp"
#include "base/atomic/atomic.hpp"
#include "base/thread/thread_pool.hpp"
#include "base/math/vector.inl"

#include <iostream>

namespace rendering { namespace film {

Film::Film(math::uint2 dimensions, float exposure, tonemapping::Tonemapper* tonemapper) :
	pixels_(new Pixel[dimensions.x * dimensions.y]),
	exposure_(exposure),
	tonemapper_(tonemapper),
	image_(image::Image::Description(image::Image::Type::Float_4, dimensions)),
	seeds_(new math::uint2[dimensions.x * dimensions.y]) {}

Film::~Film() {
	delete [] seeds_;
	delete tonemapper_;
	delete [] pixels_;
}

math::uint2 Film::dimensions() const {
	return image_.description().dimensions;
}

const image::Image_float_4& Film::resolve(thread::Pool& pool) {
	auto d = dimensions();
	pool.run_range([this](uint32_t begin, uint32_t end){ resolve(begin, end); }, 0, d.x * d.y);
	return image_;
}

void Film::clear() {
	auto d = dimensions();
	for (uint32_t i = 0, len = d.x * d.y; i < len; ++i) {
		pixels_[i].color = math::float3(0.f, 0.f, 0.f);
		pixels_[i].weight_sum = 0.f;
	}
}

math::uint2 Film::seed(uint32_t x, uint32_t y) const {
	auto d = dimensions();
	return seeds_[d.x * y + x];
}

void Film::set_seed(uint32_t x, uint32_t y, math::uint2 seed) {
	auto d = dimensions();
	seeds_[d.x * y + x] = seed;
}

void Film::add_pixel(uint32_t x, uint32_t y, const math::float3& color, float weight) {
	auto d = dimensions();
	if (x >= d.x || y >= d.y) {
		return;
	}

	auto& pixel = pixels_[d.x * y + x];
	pixel.color += weight * color;
	pixel.weight_sum += weight;
}

void Film::add_pixel_atomic(uint32_t x, uint32_t y, const math::float3& color, float weight) {
//	add_pixel(x, y, color, weight);

	auto d = dimensions();
	if (x >= d.x || y >= d.y) {
		return;
	}

	auto& pixel = pixels_[d.x * y + x];

	pixel.color += weight * color;
	atomic::add(pixel.color.x, weight * color.x);
	atomic::add(pixel.color.y, weight * color.y);
	atomic::add(pixel.color.z, weight * color.z);
	atomic::add(pixel.weight_sum, weight);
}

void Film::resolve(uint32_t begin, uint32_t end) {
	for (uint32_t i = begin; i < end; ++i) {
		auto& pixel = pixels_[i];

		math::float3 color = pixel.color / pixel.weight_sum;

		math::float3 exposed = expose(color, exposure_);

		math::float3 tonemapped = tonemapper_->tonemap(exposed);

		image_.set(i, math::float4(tonemapped, 1.f));

		if (pixel.weight_sum <= 0.f) {
			std::cout << "alarm" << std::endl;
		}
	}
}

math::float3 Film::expose(const math::float3& color, float exposure) {
	return std::exp2(exposure) * color;
}

}}
