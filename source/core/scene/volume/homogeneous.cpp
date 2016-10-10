#include "homogeneous.hpp"
#include "base/math/ray.inl"
#include "base/math/vector4.inl"
#include "base/math/bounding/aabb.inl"

namespace scene { namespace volume {

float3 Homogeneous::optical_depth(const math::Oray& ray, float /*step_size*/,
								  math::random::Generator& /*rng*/, Worker& /*worker*/,
								  Sampler_filter /*filter*/) const {
	return ray.length() * (absorption_ + scattering_);
}

float3 Homogeneous::scattering(float3_p /*p*/, Worker& /*worker*/,
							   Sampler_filter /*filter*/) const {
	return scattering_;
}

void Homogeneous::set_parameter(const std::string& /*name*/, const json::Value& /*value*/) {}

}}
