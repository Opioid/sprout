#pragma once

#include "prop_intersection.hpp"
#include "prop.hpp"

namespace scene {

inline material::Material* Intersection::material() const {
	return prop->material(geo.part);
}

inline float Intersection::opacity(Worker& worker, float time, material::Texture_filter override_filter) const {
	auto m = material();
	auto& sampler = worker.sampler(m->sampler_key(), override_filter);
	return m->opacity(geo.uv, time, sampler);
}

inline const material::Sample& Intersection::sample(Worker& worker, math::pfloat3 wo, float time,
													material::Texture_filter override_filter) const {
	auto m = material();
	auto& sampler = worker.sampler(m->sampler_key(), override_filter);
	return m->sample(geo, wo, time, 1.f, sampler, worker.id());
}

}
