#include "display_sample.hpp"
#include "scene/material/material_sample.inl"
#include "sampler/sampler.hpp"
#include "scene/material/fresnel/fresnel.inl"
#include "scene/material/ggx/ggx.inl"
#include "base/math/vector.inl"

namespace scene { namespace material { namespace display {

math::vec3 Sample::evaluate(math::pvec3 wi, float& pdf) const {
	// Roughness zero will always have zero specular term (or worse NaN)
	if (0.f == a2_) {
		pdf = 0.f;
		return math::vec3_identity;
	}

	float n_dot_wi = std::max(math::dot(n_, wi),  0.00001f);
	float n_dot_wo = std::max(math::dot(n_, wo_), 0.00001f);

	math::vec3 h = math::normalized(wo_ + wi);

	float n_dot_h  = math::dot(n_, h);
	float wo_dot_h = math::dot(wo_, h);

	float d = ggx::distribution_isotropic(n_dot_h, std::max(a2_, 0.0000001f));
	float g = ggx::geometric_shadowing(n_dot_wi, n_dot_wo, a2_);
	math::vec3 f = fresnel::schlick(wo_dot_h, f0_);

	math::vec3 specular = d * g * f;

	pdf = d * n_dot_h / (4.f * wo_dot_h);

	return n_dot_wi * specular;
}

math::vec3 Sample::emission() const {
	return emission_;
}

math::vec3 Sample::attenuation() const {
	return math::vec3(100.f, 100.f, 100.f);
}

float Sample::ior() const {
	return 1.5f;
}

void Sample::sample_evaluate(sampler::Sampler& sampler, bxdf::Result& result) const {
	if (!same_hemisphere(wo_)) {
		result.pdf = 0.f;
		return;
	}

	float n_dot_wo = clamped_n_dot_wo();

	float n_dot_wi = ggx_.importance_sample(*this, sampler, n_dot_wo, result);

	result.reflection = n_dot_wi * result.reflection;
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

void Sample::set(const math::vec3& emission, float f0, float roughness) {
	emission_ = emission;
	f0_ = math::vec3(f0);
	float a = roughness * roughness;
	a2_ = a * a;
}

}}}
