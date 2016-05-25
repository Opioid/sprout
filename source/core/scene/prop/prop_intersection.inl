#pragma once

#include "prop_intersection.hpp"
#include "prop.hpp"

namespace scene {

inline material::Material* Intersection::material() const {
	return prop->material(geo.part);
}

inline float Intersection::area() const {
	return prop->area(geo.part);
}

inline float Intersection::opacity(Worker& worker, float time,
								   Sampler_filter filter) const {
	return material()->opacity(geo.uv, time, worker, filter);
}

inline const material::Sample& Intersection::sample(Worker& worker, math::pfloat3 wo,
													float time, Sampler_filter filter) const {
	return material()->sample(geo, wo, time, 1.f, worker, filter);
}

}
