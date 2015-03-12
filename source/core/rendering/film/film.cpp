#include "film.hpp"
#include "base/math/vector.inl"

namespace film {

Film::Film(const math::uint2& dimensions) :
	pixels_(new Pixel[dimensions.x * dimensions.y]),
	image_buffer_(dimensions) {}

Film::~Film() {
	delete [] pixels_;
}

const math::uint2& Film::dimensions() const {
	return image_buffer_.dimensions();
}

const image::Buffer4& Film::resolve() {
	auto& d = image_buffer_.dimensions();
	for (uint32_t y = 0; y < d.y; ++y) {
		for (uint32_t x = 0; x < d.x; ++x) {
			auto& pixel = pixels_[dimensions().x * y + x];
			image_buffer_.set4(x, y, math::float4(pixel.color / pixel.weight_sum, 1.f));
		}
	}

	return image_buffer_;
}

void Film::add_pixel(uint32_t x, uint32_t y, const math::float3& color, float weight) {
	auto& pixel = pixels_[dimensions().x * y + x];
	pixel.color += weight * color;
	pixel.weight_sum += weight;
}

}
