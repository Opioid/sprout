#include "prop_image_light.hpp"
#include "light_sample.hpp"
#include "sampler/sampler.hpp"
#include "scene/scene_worker.hpp"
#include "scene/prop/prop.hpp"
#include "scene/shape/shape.hpp"
#include "scene/shape/shape_sample.hpp"
#include "scene/material/material.hpp"
#include "base/math/math.hpp"
#include "base/math/vector.inl"
#include "base/math/matrix.inl"
#include "base/math/bounding/aabb.inl"

namespace scene { namespace light {

void Prop_image_light::sample(const entity::Composed_transformation& transformation, float time,
							  const math::float3& p, const math::float3& n, bool total_sphere,
							  sampler::Sampler& sampler, Worker& worker,
							  material::Texture_filter override_filter, Sample& result) const {
	auto material = prop_->material(part_);

	float pdf;
	math::float2 uv = material->emission_importance_sample(sampler.generate_sample_2D(), pdf);

	prop_->shape()->sample(part_, transformation, area_, p, uv, result.shape);

	if (math::dot(result.shape.wi, n) > 0.f || total_sphere) {
		auto& image_sampler = worker.sampler(material->sampler_key(), override_filter);
		result.shape.pdf *= pdf;
		result.energy = material->sample_emission(result.shape.uv, time, image_sampler);
	} else {
		result.shape.pdf = 0.f;
	}
}

float Prop_image_light::pdf(const entity::Composed_transformation& transformation,
							const math::float3& p, const math::float3& wi, bool /*total_sphere*/,
							Worker& worker, material::Texture_filter override_filter) const {
	shape::Sample sample;
	prop_->shape()->sample(part_, transformation, area_, p, wi, sample);

	auto material = prop_->material(part_);
	auto& sampler = worker.sampler(material->sampler_key(), override_filter);
	float pdf = material->emission_pdf(sample.uv, sampler);

	return sample.pdf * pdf;
}

void Prop_image_light::prepare_sampling() {
	prop_->material(part_)->prepare_sampling(true);

	entity::Composed_transformation temp;
	auto& transformation = prop_->transformation_at(0.f, temp);
	area_ = prop_->shape()->area(part_, math::float3(transformation.scale));
}

}}
