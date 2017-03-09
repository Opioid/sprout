#pragma once

#include "scene_intersection.hpp"
#include "scene_renderstate.hpp"

namespace scene {

inline float Intersection::opacity(const Worker& worker, float time, Sampler_filter filter) const {
	return material()->opacity(geo.uv, time, worker, filter);
}

inline float3 Intersection::thin_absorption(const Worker& worker, float3_p wo,
											float time, Sampler_filter filter) const {
	return material()->thin_absorption(wo, geo.geo_n, geo.uv, time, worker, filter);
}

inline const material::Sample& Intersection::sample(const Worker& worker, float3_p wo,
													float time, Sampler_filter filter) const {
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

}
