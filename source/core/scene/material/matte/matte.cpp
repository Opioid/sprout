#include "matte.hpp"
#include "scene/material/lambert/lambert.inl"
//#include "scene/material/oren_nayar/oren_nayar.inl"
#include "sampler/sampler.hpp"
#include "base/math/math.hpp"
#include "base/math/vector.inl"
#include "base/math/sampling/sampling.inl"

namespace scene { namespace material { namespace matte {

Sample::Sample() : lambert_(*this)/*, oren_nayar_(*this)*/ {}

math::float3 Sample::evaluate(const math::float3& wi, float& pdf) const {
	float n_dot_wi = std::max(math::dot(n_, wi),  0.00001f);
	pdf = n_dot_wi * math::Pi_inv;
	return pdf * diffuse_color_;

//	float n_dot_wo = clamped_n_dot_wo();
//	pdf = n_dot_wi * math::Pi_inv;
//	math::float3 result = oren_nayar_.evaluate(wi, n_dot_wi, n_dot_wo);
//	// Pi_inv is already in the result
//	return n_dot_wi * result;
}

math::float3 Sample::emission() const {
	return math::float3::identity;
}

math::float3 Sample::attenuation() const {
	return math::float3(100.f, 100.f, 100.f);
}

void Sample::sample_evaluate(sampler::Sampler& sampler, BxDF_result& result) const {
	if (!same_hemisphere(wo_)) {
		result.pdf = 0.f;
		return;
	}

	float n_dot_wi = lambert_.importance_sample(sampler, result);
	result.reflection *= n_dot_wi;

//	float n_dot_wi = oren_nayar_.importance_sample(sampler, clamped_n_dot_wo(), result);
//	result.reflection *= n_dot_wi;
}

bool Sample::is_pure_emissive() const {
	return false;
}

void Sample::set(const math::float3& color, float sqrt_roughness) {
	diffuse_color_ = color;

	float roughness = sqrt_roughness * sqrt_roughness;
	a2_ = roughness * roughness;
}

Matte::Matte(Sample_cache<Sample>& cache, std::shared_ptr<image::texture::Texture_2D> mask, bool two_sided) :
	Material(cache, mask, two_sided) {}

math::float3 Matte::sample_emission(math::float2 /*uv*/,
									const image::texture::sampler::Sampler_2D& /*sampler*/) const {
	return math::float3::identity;
}

math::float3 Matte::average_emission() const {
	return math::float3::identity;
}

const image::texture::Texture_2D* Matte::emission_map() const {
	return nullptr;
}

}}}

