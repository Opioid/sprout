#pragma once

#include "shape/geometry/shape_intersection.hpp"
#include "material/sampler_settings.hpp"

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

	const material::Sample& sample(Worker& worker, float3_p wo, float time,
								   Sampler_filter filter) const;

	const Prop* prop;
	shape::Intersection geo;
};

}
