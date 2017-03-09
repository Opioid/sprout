#pragma once

#include "prop.hpp"
#include "shape/shape_intersection.hpp"
#include "material/sampler_settings.hpp"
#include "base/math/vector3.inl"

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

	bool hit() const {
		return nullptr != prop;
	}

	material::Material* material() const{
		return prop->material(geo.part);
	}

	uint32_t light_id() const {
		return prop->light_id(geo.part);
	}

	float area() const {
		return prop->area(geo.part);
	}

	float opacity(const Worker& worker, float time, Sampler_filter filter) const;

	float3 thin_absorption(const Worker& worker, float3_p wo,
						   float time, Sampler_filter filter) const;

	const material::Sample& sample(const Worker& worker, float3_p wo,
								   float time, Sampler_filter filter) const;

	const material::BSSRDF& bssrdf(const Worker& worker) const {
		return material()->bssrdf(worker);
	}

	bool same_hemisphere(float3_p v) const {
		return math::dot(geo.geo_n, v) > 0.f;
	}

	shape::Intersection geo;
	const Prop* prop;
};

}
