#include "sensor.hpp"
#include "image/typed_image.inl"
#include "tonemapping/tonemapper.hpp"
#include "base/math/vector.inl"
#include "base/thread/thread_pool.hpp"

namespace rendering { namespace sensor {

Sensor::Sensor(int2 dimensions, const tonemapping::Tonemapper* tonemapper) :
	dimensions_(dimensions),
	tonemapper_(tonemapper) {}

Sensor::~Sensor() {
	delete tonemapper_;
}

int2 Sensor::dimensions() const {
	return dimensions_;
}

void Sensor::resolve(thread::Pool& pool, image::Image_float_4& target) const {
	pool.run_range([this, &target](int32_t begin, int32_t end) {
		resolve(begin, end, target); }, 0, target.area());
}

}}
