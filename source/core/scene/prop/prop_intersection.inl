#pragma once

#include "prop_intersection.hpp"
#include "prop.hpp"

namespace scene {

inline material::IMaterial* Intersection::material() const {
	return prop->material(geo.part);
}

inline float Intersection::opacity(float time, const image::texture::sampler::Sampler_2D& sampler) const {
	return material()->opacity(geo.uv, time, sampler);
}

}
