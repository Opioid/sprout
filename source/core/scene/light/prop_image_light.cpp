#include "prop_image_light.hpp"
#include "light_sample.hpp"
#include "sampler/sampler.hpp"
#include "scene/prop.hpp"
#include "scene/scene_worker.hpp"
#include "scene/shape/shape.hpp"
#include "scene/shape/shape_sample.hpp"
#include "scene/material/material.hpp"
#include "base/math/math.hpp"
#include "base/math/vector.inl"
#include "base/math/matrix.inl"
#include "base/math/bounding/aabb.inl"

namespace scene { namespace light {

void Prop_image_light::sample(const Transformation& transformation, float time,
							  float3_p p, float3_p n, bool total_sphere,
							  sampler::Sampler& sampler, Worker& worker,
							  Sampler_filter filter, Sample& result) const {
	auto material = prop_->material(part_);

	float area = prop_->area(part_);

	float pdf;
	float2 uv = material->radiance_importance_sample(sampler.generate_sample_2D(), pdf);

	prop_->shape()->sample(part_, transformation, area, p, uv, result.shape);

	if (math::dot(result.shape.wi, n) > 0.f || total_sphere) {
		result.shape.pdf *= pdf;
		result.radiance = material->sample_radiance(result.shape.wi, result.shape.uv,
													area, time, worker, filter);
	} else {
		result.shape.pdf = 0.f;
	}
}

float Prop_image_light::pdf(const Transformation& transformation,
							float3_p p, float3_p wi, bool /*total_sphere*/,
							Worker& worker, Sampler_filter filter) const {
	float area = prop_->area(part_);

	shape::Sample sample;
	prop_->shape()->sample(part_, transformation, area, p, wi, sample);

	float pdf = prop_->material(part_)->emission_pdf(sample.uv, worker, filter);

	return sample.pdf * pdf;
}

void Prop_image_light::prepare_sampling() {
	prop_->material(part_)->prepare_sampling(true);

	prop_->prepare_sampling(part_);
}

}}
