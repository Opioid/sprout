#pragma once

#include "shape/shape_intersection.hpp"
#include "material/sampler_settings.hpp"

namespace scene {

class Worker;

namespace material {

class BSSRDF;
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

	float opacity(const Worker& worker, float time, Sampler_filter filter) const;

	float3 thin_absorption(const Worker& worker, float3_p wo,
						   float time, Sampler_filter filter) const;

	const material::Sample& sample(const Worker& worker, float3_p wo,
								   float time, Sampler_filter filter) const;

	const material::BSSRDF& bssrdf(const Worker& worker) const;

	bool same_hemisphere(float3_p v) const;

	shape::Intersection geo;
	const Prop* prop;
};

}
