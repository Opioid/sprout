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
	using Sampler_filter = material::Sampler_settings::Filter;

	material::Material* material() const;

	float area() const;

	float opacity(Worker& worker, float time, Sampler_filter filter) const;

	const material::Sample& sample(Worker& worker, math::pfloat3 wo,
								   float time, Sampler_filter filter) const;

	const Prop* prop;
	shape::Intersection geo;
};

}
