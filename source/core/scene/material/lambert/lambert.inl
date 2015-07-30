#include "lambert.hpp"
#include "sampler/sampler.hpp"
#include "base/math/sampling/sampling.inl"
#include "base/math/vector.inl"
#include "base/math/math.hpp"

namespace scene { namespace material { namespace lambert {

template<typename Sample>
Lambert<Sample>::Lambert(const Sample& sample) : BxDF<Sample>(sample) {}

template<typename Sample>
math::float3 Lambert<Sample>::evaluate(const math::float3& /*wi*/, float /*n_dot_wi*/) const {
	return math::Pi_inv * BxDF<Sample>::sample_.diffuse_color_;
}

template<typename Sample>
float Lambert<Sample>::pdf(const math::float3& /*wi*/, float n_dot_wi) const {
	return n_dot_wi * math::Pi_inv;
}

template<typename Sample>
float Lambert<Sample>::importance_sample(sampler::Sampler& sampler, BxDF_result& result) const {
	math::float2 s2d = sampler.generate_sample_2D();

	math::float3 is = math::sample_hemisphere_cosine(s2d);
	result.wi = math::normalized(BxDF<Sample>::sample_.tangent_to_world(is));

//	result.pdf = 1.f;
//	result.reflection = BxDF<Sample>::sample_.diffuse_color_;

	float n_dot_wi = std::max(math::dot(BxDF<Sample>::sample_.n_, result.wi),  0.00001f);
	result.pdf = n_dot_wi * math::Pi_inv;
	result.reflection = math::Pi_inv * BxDF<Sample>::sample_.diffuse_color_;

	result.type.clear_set(BxDF_type::Diffuse_reflection);

	return n_dot_wi;
}

}}}
