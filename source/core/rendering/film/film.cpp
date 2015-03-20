#include "film.hpp"
#include "tonemapping/tonemapper.hpp"
#include "base/math/vector.inl"

namespace rendering { namespace film {

Film::Film(const math::uint2& dimensions, float exposure, tonemapping::Tonemapper* tonemapper) :
	pixels_(new Pixel[dimensions.x * dimensions.y]),
	exposure_(exposure),
	tonemapper_(tonemapper),
	image_buffer_(dimensions) {
	clear();
}

Film::~Film() {
	delete tonemapper_;
	delete [] pixels_;
}

const math::uint2& Film::dimensions() const {
	return image_buffer_.dimensions();
}

const image::Buffer4& Film::resolve() {
	auto& d = dimensions();
	for (uint32_t y = 0; y < d.y; ++y) {
		for (uint32_t x = 0; x < d.x; ++x) {
			auto& pixel = pixels_[d.x * y + x];

			math::float3 color = pixel.color / pixel.weight_sum;

			math::float3 exposed = expose(color, exposure_);

			math::float3 tonemapped = tonemapper_->tonemap(exposed);

			image_buffer_.set4(x, y, math::float4(tonemapped, 1.f));
		}
	}

	return image_buffer_;
}

void Film::clear() {
	uint32_t len = dimensions().x * dimensions().y;
	for (uint32_t i = 0; i < len; ++i) {
		pixels_[i].color = math::float3(0.f, 0.f, 0.f);
		pixels_[i].weight_sum = 0.f;
	}
}

void Film::add_pixel(uint32_t x, uint32_t y, const math::float3& color, float weight) {
	const math::uint2& d = dimensions();

	if (x >= d.x || y >= d.y) {
		return;
	}

	auto& pixel = pixels_[d.x * y + x];
	pixel.color += weight * color;
	pixel.weight_sum += weight;
}

math::float3 Film::expose(const math::float3& color, float exposure) {
	return std::exp2(exposure) * color;
}

}}
