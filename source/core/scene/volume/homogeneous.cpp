#include "homogeneous.hpp"
#include "base/math/aabb.inl"
#include "base/math/vector3.inl"
#include "base/math/ray.inl"

namespace scene { namespace volume {

float3 Homogeneous::optical_depth(const math::Ray& ray, float /*step_size*/,
								  rnd::Generator& /*rng*/, Worker& /*worker*/,
								  Sampler_filter /*filter*/) const {
	return ray.length() * (absorption_ + scattering_);
}

float3 Homogeneous::scattering(const float3& /*p*/, Worker& /*worker*/,
							   Sampler_filter /*filter*/) const {
	return scattering_;
}

void Homogeneous::set_parameter(const std::string& /*name*/, const json::Value& /*value*/) {}

}}
