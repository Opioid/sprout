#include "postprocessor_pipeline.hpp"
#include "postprocessor.hpp"
#include "image/typed_image.inl"
#include "rendering/sensor/sensor.hpp"
#include "base/math/vector.inl"

namespace rendering { namespace postprocessor {

Pipeline::Pipeline() {}

Pipeline::~Pipeline() {
	for (auto pp : postprocessors_) {
		delete pp;
	}
}

void Pipeline::reserve(size_t num_pps) {
	postprocessors_.reserve(num_pps);
}

void Pipeline::add(Postprocessor* pp) {
	postprocessors_.push_back(pp);
}

void Pipeline::init(const scene::camera::Camera& camera) {
	if (postprocessors_.empty()) {
		return;
	}

	image::Image::Description description(image::Image::Type::Float_4, camera.sensor_dimensions());
	scratch_.resize(description);

	for (auto pp : postprocessors_) {
		pp->init(camera);
	}
}

void Pipeline::apply(const sensor::Sensor& sensor, image::Image_float_4& target,
					 thread::Pool& pool) {
	if (postprocessors_.empty()) {
		sensor.resolve(pool, target);
	} else {
		sensor.resolve(pool, scratch_);

		for (auto pp : postprocessors_) {
			pp->apply(scratch_, target, pool);
		}
	}
}

}}
