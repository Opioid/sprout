#include "sensor.hpp"
#include "image/typed_image.inl"
#include "tonemapping/tonemapper.hpp"
#include "base/math/vector.inl"
#include "base/thread/thread_pool.hpp"

namespace rendering { namespace sensor {

Sensor::Sensor(math::int2 dimensions, float exposure, const tonemapping::Tonemapper* tonemapper) :
	dimensions_(dimensions),
	exp2_exposure_(std::exp2(exposure)),
	tonemapper_(tonemapper) {}

Sensor::~Sensor() {
	delete tonemapper_;
}

math::int2 Sensor::dimensions() const {
	return dimensions_;
}

void Sensor::resolve(thread::Pool& pool, image::Image_float_4& target) {
	auto d = target.description().dimensions;
	pool.run_range([this, &target](int32_t begin, int32_t end){ resolve(begin, end, target); }, 0, d.x * d.y);
}

math::vec3 Sensor::expose(math::pvec3 color) const {
	return exp2_exposure_ * color;
}

}}
