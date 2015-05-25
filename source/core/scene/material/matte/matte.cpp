#include "matte.hpp"
#include "scene/material/lambert/lambert.inl"
#include "sampler/sampler.hpp"
#include "base/math/sampling.hpp"
#include "base/math/vector.inl"
#include "base/math/math.hpp"

namespace scene { namespace material { namespace matte {

Sample::Sample() : lambert_(*this) {}

math::float3 Sample::evaluate(const math::float3& wi, float& pdf) const {
	float n_dot_wi = std::max(math::dot(n_, wi),  0.00001f);
	pdf = n_dot_wi * math::Pi_inv;
	return pdf * diffuse_color_;
}

math::float3 Sample::emission() const {
	return math::float3::identity;
}

math::float3 Sample::attenuation() const {
	return math::float3(1.f, 1.f, 1.f);
}

void Sample::sample_evaluate(sampler::Sampler& sampler, BxDF_result& result) const {
	if (!same_hemisphere(wo_)) {
		result.pdf = 0.f;
		return;
	}

	lambert_.importance_sample(sampler, result);
}

void Sample::set(const math::float3& color) {
	diffuse_color_ = color;
}

Matte::Matte(Sample_cache<Sample>& cache, std::shared_ptr<image::Image> mask) : Material(cache, mask) {}

math::float3 Matte::sample_emission() const {
	return math::float3::identity;
}

math::float3 Matte::average_emission() const {
	return math::float3::identity;
}

}}}

