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

	bool hit() const;

	material::Material* material() const;

	uint32_t light_id() const;

	float area() const;

	float opacity(Worker& worker, float time, Sampler_filter filter) const;

	const material::Sample& sample(Worker& worker, float3_p wo, float time,
								   Sampler_filter filter) const;

	const Prop* prop;
	shape::Intersection geo;
};

}
