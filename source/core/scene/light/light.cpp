#include "light.hpp"
#include "scene/entity/composed_transformation.hpp"
#include "base/math/vector.inl"
#include "base/math/matrix.inl"

namespace scene { namespace light {

void Light::sample(float time, const math::float3& p, const math::float3& n, bool total_sphere,
				   sampler::Sampler& sampler, Worker& worker,
				   material::Texture_filter override_filter, Sample& result) const {
	entity::Composed_transformation temp;
	auto& transformation = transformation_at(time, temp);

	sample(transformation, time, p, n, total_sphere, sampler, worker, override_filter, result);
}

void Light::sample(float time, const math::float3& p, sampler::Sampler& sampler,
				   Worker& worker, material::Texture_filter override_filter, Sample& result) const {
	sample(time, p, math::float3_identity, true, sampler, worker, override_filter, result);
}

}}

