#pragma once

#include "scene_intersection.hpp"
#include "scene_renderstate.hpp"
#include "prop.hpp"

namespace scene {

inline bool Intersection::hit() const {
	return nullptr != prop;
}

inline material::Material* Intersection::material() const {
	return prop->material(geo.part);
}

inline float Intersection::area() const {
	return prop->area(geo.part);
}

inline uint32_t Intersection::light_id() const {
	return prop->light_id(geo.part);
}

inline float Intersection::opacity(const Worker& worker, float time, Sampler_filter filter) const {
	return material()->opacity(geo.uv, time, worker, filter);
}

inline const material::Sample& Intersection::sample(const Worker& worker, float3_p wo, float time,
													Sampler_filter filter) const {
	material::Material* material = Intersection::material();

	Renderstate rs;
	rs.p = geo.p;
	rs.t = geo.t;
	rs.b = geo.b;

	if (material->is_two_sided() && !same_hemisphere(wo)) {
		rs.n     = -geo.n;
		rs.geo_n = -geo.geo_n;
	} else {
		rs.n     = geo.n;
		rs.geo_n = geo.geo_n;
	}

	rs.uv = geo.uv;

	rs.area = area();
	rs.time = time;
	rs.ior  = 1.f;

	return material->sample(wo, rs, worker, filter);
}

inline const material::BSSRDF& Intersection::bssrdf(const Worker& worker) const {
	return material()->bssrdf(worker);
}

inline bool Intersection::same_hemisphere(float3_p v) const {
	return math::dot(geo.geo_n, v) > 0.f;
}

}
