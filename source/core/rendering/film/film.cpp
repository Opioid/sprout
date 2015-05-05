#include "film.hpp"
#include "tonemapping/tonemapper.hpp"
#include "base/thread/thread_pool.hpp"
#include "base/math/vector.inl"

namespace rendering { namespace film {

Film::Film(const math::uint2& dimensions, float exposure, tonemapping::Tonemapper* tonemapper) :
	pixels_(new Pixel[dimensions.x * dimensions.y]),
	exposure_(exposure),
	tonemapper_(tonemapper),
	image_(image::Description(dimensions)) {
	clear();
}

Film::~Film() {
	delete tonemapper_;
	delete [] pixels_;
}

const math::uint2& Film::dimensions() const {
	return image_.description().dimensions;
}

const image::Image& Film::resolve(thread::Pool& pool) {
	auto& d = dimensions();
	pool.run_range([this](uint32_t begin, uint32_t end){ resolve(begin, end); }, 0, d.x * d.y);
	return image_;
}

void Film::clear() {
	auto& d = dimensions();
	for (uint32_t i = 0, len = d.x * d.y; i < len; ++i) {
		pixels_[i].color = math::float3(0.f, 0.f, 0.f);
		pixels_[i].weight_sum = 0.f;
	}
}

void Film::add_pixel(uint32_t x, uint32_t y, const math::float3& color, float weight) {
	auto& d = dimensions();
	if (x >= d.x || y >= d.y) {
		return;
	}

	auto& pixel = pixels_[d.x * y + x];
	pixel.color += weight * color;
	pixel.weight_sum += weight;
}

void Film::add_pixel_atomic(uint32_t x, uint32_t y, const math::float3& color, float weight) {
	add_pixel(x, y, color, weight);
}

void Film::resolve(uint32_t begin, uint32_t end) {
	for (uint32_t i = begin; i < end; ++i) {
		auto& pixel = pixels_[i];

		math::float3 color = pixel.color / pixel.weight_sum;

		math::float3 exposed = expose(color, exposure_);

		math::float3 tonemapped = tonemapper_->tonemap(exposed);

		image_.set4(i, math::float4(tonemapped, 1.f));
	}
}

math::float3 Film::expose(const math::float3& color, float exposure) {
	return std::exp2(exposure) * color;
}

}}
