#include "light.hpp"
#include "scene/entity/composed_transformation.hpp"
#include "base/math/vector.inl"
#include "base/math/matrix.inl"

namespace scene { namespace light {

void Light::sample(float time, const math::float3& p, const math::float3& n, bool total_sphere,
				   const image::texture::sampler::Sampler_2D& image_sampler,
				   sampler::Sampler& sampler, shape::Node_stack& node_stack, Sample& result) const {
	entity::Composed_transformation temp;
	auto& transformation = transformation_at(time, temp);

	sample(transformation, time, p, n, total_sphere, image_sampler, sampler, node_stack, result);
}

void Light::sample(float time, const math::float3& p, const image::texture::sampler::Sampler_2D& image_sampler,
				   sampler::Sampler& sampler, shape::Node_stack& node_stack, Sample& result) const {
	sample(time, p, math::float3_identity, true, image_sampler, sampler, node_stack, result);
}

}}

