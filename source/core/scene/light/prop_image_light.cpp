#include "prop_image_light.hpp"
#include "light_sample.hpp"
#include "sampler/sampler.hpp"
#include "scene/prop.hpp"
#include "scene/scene_worker.hpp"
#include "scene/shape/shape.hpp"
#include "scene/shape/shape_sample.hpp"
#include "scene/material/material.hpp"
#include "base/math/aabb.inl"
#include "base/math/math.hpp"
#include "base/math/vector3.inl"
#include "base/math/matrix4x4.inl"

namespace scene { namespace light {

void Prop_image_light::sample(const Transformation& transformation, float time,
							  float3_p p, float3_p n, bool total_sphere,
							  sampler::Sampler& sampler, uint32_t sampler_dimension,
							  Worker& worker, Sampler_filter filter, Sample& result) const {
	const auto material = prop_->material(part_);

	const float2 s2d = sampler.generate_sample_2D(sampler_dimension);

	float material_pdf;
	const float2 uv = material->radiance_sample(s2d, material_pdf);

	const float area = prop_->area(part_);

	const bool two_sided = material->is_two_sided();

	// this pdf includes the uv weight which adjusts for texture distortion by the shape
	prop_->shape()->sample(part_, transformation, p, uv, area, two_sided, result.shape);

	if (math::dot(result.shape.wi, n) > 0.f || total_sphere) {
		result.shape.pdf *= material_pdf;
		result.radiance = material->sample_radiance(result.shape.wi, uv,
													area, time, worker, filter);
	} else {
		result.shape.pdf = 0.f;
	}
}

float Prop_image_light::pdf(const Transformation& transformation,
							float3_p p, float3_p wi, bool /*total_sphere*/,
							Worker& worker, Sampler_filter filter) const {
	const float area = prop_->area(part_);

	const auto material = prop_->material(part_);

	const bool two_sided = material->is_two_sided();

	// this pdf includes the uv weight which adjusts for texture distortion by the shape
	float2 uv;
	float shape_pdf = prop_->shape()->pdf_uv(part_, transformation, p, wi, area, two_sided, uv);

	const float material_pdf = material->emission_pdf(uv, worker, filter);

	return shape_pdf * material_pdf;
}

void Prop_image_light::prepare_sampling(uint32_t light_id, thread::Pool& pool) {
	prop_->prepare_sampling(part_, light_id, true, pool);
}

}}
