#include "light.hpp"
#include "scene/entity/composed_transformation.hpp"
#include "base/math/vector.inl"
#include "base/math/matrix.inl"

namespace scene { namespace light {

void Light::sample(float time, float3_p p, float3_p n, bool total_sphere,
				   sampler::Sampler& sampler, uint32_t sampler_dimension,
				   Worker& worker, Sampler_filter filter, Sample& result) const {
	entity::Composed_transformation temp;
	const auto& transformation = transformation_at(time, temp);

	sample(transformation, time, p, n, total_sphere,
		   sampler, sampler_dimension, worker, filter, result);
}

void Light::sample(float time, float3_p p,
				   sampler::Sampler& sampler, uint32_t sampler_dimension,
				   Worker& worker, Sampler_filter filter, Sample& result) const {
	sample(time, p, float3::identity(), true,
		   sampler, sampler_dimension, worker, filter, result);
}

float Light::pdf(float time, float3_p p, float3_p wi, bool total_sphere,
				 Worker& worker, Sampler_filter filter) const {
	entity::Composed_transformation temp;
	const auto& transformation = transformation_at(time, temp);

	return pdf(transformation, p, wi, total_sphere, worker, filter);
}

}}

