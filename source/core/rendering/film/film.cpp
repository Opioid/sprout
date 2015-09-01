#include "film.hpp"
#include "image/typed_image.inl"
#include "tonemapping/tonemapper.hpp"
#include "base/thread/thread_pool.hpp"
#include "base/math/vector.inl"

namespace rendering { namespace film {

Film::Film(math::uint2 dimensions, float exposure, tonemapping::Tonemapper* tonemapper) :
	exposure_(exposure),
	tonemapper_(tonemapper),
	image_(image::Image::Description(image::Image::Type::Float_4, dimensions)),
	seeds_(new math::uint2[dimensions.x * dimensions.y]) {}

Film::~Film() {
	delete [] seeds_;
	delete tonemapper_;
}

math::uint2 Film::dimensions() const {
	return image_.description().dimensions;
}

const image::Image_float_4& Film::resolve(thread::Pool& pool) {
	auto d = dimensions();
	pool.run_range([this](uint32_t begin, uint32_t end){ resolve(begin, end); }, 0, d.x * d.y);
	return image_;
}

math::uint2 Film::seed(uint32_t x, uint32_t y) const {
	auto d = dimensions();
	return seeds_[d.x * y + x];
}

void Film::set_seed(uint32_t x, uint32_t y, math::uint2 seed) {
	auto d = dimensions();
	seeds_[d.x * y + x] = seed;
}

math::float3 Film::expose(const math::float3& color, float exposure) {
	return std::exp2(exposure) * color;
}

}}
