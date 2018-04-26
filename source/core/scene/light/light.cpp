#include "light.hpp"
#include "scene/entity/composed_transformation.hpp"
#include "base/math/vector3.inl"
#include "base/math/matrix4x4.inl"

namespace scene::light {

bool Light::sample(f_float3 p, f_float3 n, float time, bool total_sphere,
				   sampler::Sampler& sampler, uint32_t sampler_dimension,
				   Sampler_filter filter, const Worker& worker, Sample& result) const {
	entity::Composed_transformation temp;
	const auto& transformation = transformation_at(time, temp);

	return sample(p, n, time, transformation, total_sphere,
				  sampler, sampler_dimension, filter, worker, result);
}

bool Light::sample(f_float3 p, float time, sampler::Sampler& sampler, uint32_t sampler_dimension,
				   Sampler_filter filter, const Worker& worker, Sample& result) const {
	return sample(p, float3::identity(), time, true, sampler,
				  sampler_dimension, filter, worker, result);
}

bool Light::is_light(uint32_t id) {
	return 0xFFFFFFFF != id;
}

}
