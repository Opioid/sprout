#pragma once

#include "scene/shape/geometry/shape_intersection.hpp"
#include "scene/material/sampler_settings.hpp"

namespace scene {

class Worker;

namespace material {

class Sample;
class Material;

}

class Prop;

struct Intersection {
	material::Material* material() const;

	float opacity(Worker& worker, float time, material::Sampler_settings::Filter filter) const;

	const material::Sample& sample(Worker& worker, math::pfloat3 wo, float time,
								   material::Sampler_settings::Filter filter) const;

	const Prop* prop;
	shape::Intersection geo;
};

}
