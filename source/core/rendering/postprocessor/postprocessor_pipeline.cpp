#include "postprocessor_pipeline.hpp"
#include "rendering/sensor/sensor.hpp"
#include "base/math/vector.inl"

namespace rendering { namespace postprocessor {

void Pipeline::apply(const sensor::Sensor& sensor, image::Image_float_4& target,
					 thread::Pool& pool) const {
	sensor.resolve(pool, target);
}

}}
