#include "light_material.hpp"
#include "sampler/sampler.hpp"
#include "base/math/vector.inl"

namespace scene { namespace material { namespace light {

Sample::Sample() {}

math::float3 Sample::evaluate(const math::float3& /*wi*/, float& pdf) const {
	pdf = 0.f;
	return math::float3::identity;
}

math::float3 Sample::emission() const {
	return emission_;
}

math::float3 Sample::attenuation() const {
	return math::float3(1.f, 1.f, 1.f);
}

void Sample::sample_evaluate(sampler::Sampler& /*sampler*/, BxDF_result& result) const {
	result.reflection = math::float3::identity;
	result.pdf = 0.f;

	result.type.clear_set(BxDF_type::Diffuse_reflection);
}

void Sample::set(const math::float3& emission) {
	emission_ = emission;
}

Light::Light(Sample_cache<Sample>& cache, std::shared_ptr<image::Image> mask) : Material(cache, mask) {}

}}}
