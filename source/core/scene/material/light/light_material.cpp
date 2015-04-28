#include "light_material.hpp"
#include "sampler/sampler.hpp"
#include "base/math/vector.inl"

namespace scene { namespace material { namespace light {

Sample::Sample() {}

math::float3 Sample::evaluate(const math::float3& /*wi*/) const {
	return math::float3::identity;
}

void Sample::sample_evaluate(sampler::Sampler& sampler, Result& result) const {
	result.emission = emission_;
}

void Sample::set(const math::float3& emission) {
	emission_ = emission;
}

Light::Light(Sample_cache<Sample>& cache) : Material(cache) {}

}}}
