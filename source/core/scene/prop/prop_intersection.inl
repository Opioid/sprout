#ifndef SU_CORE_SCENE_PROP_INTERSECTION_INL
#define SU_CORE_SCENE_PROP_INTERSECTION_INL

#include "prop_intersection.hpp"
#include "prop.hpp"
#include "scene/scene_renderstate.hpp"
#include "base/math/vector3.inl"

namespace scene::prop {

inline bool Intersection::hit() const {
	return nullptr != prop;
}

inline material::Material* Intersection::material() const{
	return prop->material(geo.part);
}

inline uint32_t Intersection::light_id() const {
	return prop->light_id(geo.part);
}

inline float Intersection::area() const {
	return prop->area(geo.part);
}

inline float Intersection::opacity(float time, Sampler_filter filter, const Worker& worker) const {
	return material()->opacity(geo.uv, time, filter, worker);
}

inline float3 Intersection::thin_absorption(const float3& wo, float time,
											Sampler_filter filter, Worker& worker) const {
	return material()->thin_absorption(wo, geo.geo_n, geo.uv, time, filter, worker);
}

inline const material::Sample& Intersection::sample(const float3& wo, float time,
													Sampler_filter filter, Worker& worker) const {
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
	rs.inside_volume = inside_volume;

	return material->sample(wo, rs, filter, worker);
}

inline bool Intersection::same_hemisphere(const float3& v) const {
	return math::dot(geo.geo_n, v) > 0.f;
}

}

#endif
