#include "matte_sample.hpp"
#include "scene/material/lambert/lambert.inl"
#include "base/math/vector.inl"

namespace scene { namespace material { namespace matte {

math::float3 Sample::evaluate(math::pfloat3 wi, float& pdf) const {
	float n_dot_wi = std::max(math::dot(n_, wi),  0.00001f);

	return n_dot_wi * lambert::Isotropic::evaluate(wi, n_dot_wi, *this, pdf);
}

math::float3 Sample::radiance() const {
	return math::float3_identity;
}

math::float3 Sample::attenuation() const {
	return math::float3(100.f, 100.f, 100.f);
}

float Sample::ior() const {
	return 1.5f;
}

void Sample::sample_evaluate(sampler::Sampler& sampler, bxdf::Result& result) const {
	float n_dot_wi = lambert::Isotropic::importance_sample(*this, sampler, result);
	result.reflection *= n_dot_wi;
}

bool Sample::is_pure_emissive() const {
	return false;
}

bool Sample::is_transmissive() const {
	return false;
}

bool Sample::is_translucent() const {
	return false;
}

void Sample::set(math::pfloat3 color) {
	diffuse_color_ = color;
}

}}}
