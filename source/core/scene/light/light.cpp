#include "light.hpp"
#include "scene/entity/composed_transformation.hpp"
#include "base/math/vector3.inl"
#include "base/math/matrix4x4.inl"

namespace scene { namespace light {

void Light::sample(const float3& p, const float3& n, float time, bool total_sphere,
				   sampler::Sampler& sampler, uint32_t sampler_dimension,
				   Worker& worker, Sampler_filter filter, Sample& result) const {
	entity::Composed_transformation temp;
	const auto& transformation = transformation_at(time, temp);

	sample(transformation, p, n, time, total_sphere,
		   sampler, sampler_dimension, worker, filter, result);
}

void Light::sample(const float3& p, float time,
				   sampler::Sampler& sampler, uint32_t sampler_dimension,
				   Worker& worker, Sampler_filter filter, Sample& result) const {
	sample(p, float3::identity(), time, true,
		   sampler, sampler_dimension, worker, filter, result);
}

bool Light::is_light(uint32_t id) {
	return 0xFFFFFFFF != id;
}

}}
