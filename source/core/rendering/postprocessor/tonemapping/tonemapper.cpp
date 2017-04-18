#include "tonemapper.hpp"

namespace rendering { namespace postprocessor { namespace tonemapping {

Tonemapper::~Tonemapper() {}

void Tonemapper::init(const scene::camera::Camera& /*camera*/, thread::Pool& /*pool*/) {}

size_t Tonemapper::num_bytes() const {
	return 0;
}

float Tonemapper::normalization_factor(float linear_max, float tonemapped_max) {
	return linear_max > 0.f ? 1.f / tonemapped_max : 1.f;
}

}}}
