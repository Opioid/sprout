#include "sensor.hpp"
#include "image/typed_image.inl"
#include "tonemapping/tonemapper.hpp"
#include "base/thread/thread_pool.hpp"
#include "base/math/vector.inl"

namespace rendering { namespace sensor {

Sensor::Sensor(math::uint2 dimensions, float exposure, std::unique_ptr<tonemapping::Tonemapper> tonemapper) :
	exposure_(exposure),
	tonemapper_(std::move(tonemapper)),
	image_(image::Image::Description(image::Image::Type::Float_4, dimensions)) {}

Sensor::~Sensor() {}

math::uint2 Sensor::dimensions() const {
	return image_.description().dimensions;
}

const image::Image_float_4& Sensor::resolve(thread::Pool& pool) {
	auto d = dimensions();
	pool.run_range([this](uint32_t begin, uint32_t end){ resolve(begin, end); }, 0, d.x * d.y);
	return image_;
}

math::float3 Sensor::expose(const math::float3& color, float exposure) {
	return std::exp2(exposure) * color;
}

}}
