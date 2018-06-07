#include "prop_image_light.hpp"
#include "light_sample.hpp"
#include "sampler/sampler.hpp"
#include "scene/scene_ray.hpp"
#include "scene/scene_worker.hpp"
#include "scene/prop/prop.hpp"
#include "scene/shape/shape.hpp"
#include "scene/shape/shape_intersection.hpp"
#include "scene/shape/shape_sample.hpp"
#include "scene/material/material.hpp"
#include "base/math/aabb.inl"
#include "base/math/math.hpp"
#include "base/math/vector3.inl"
#include "base/math/matrix4x4.inl"

namespace scene::light {

bool Prop_image_light::sample(f_float3 p, float time, Transformation const& transformation,
							  sampler::Sampler& sampler, uint32_t sampler_dimension,
							  Sampler_filter filter, Worker const& worker, Sample& result) const {
	auto const material = prop_->material(part_);

	float2 const s2d = sampler.generate_sample_2D(sampler_dimension);

	auto const rs = material->radiance_sample(s2d);
	if (0.f == rs.pdf) {
		return false;
	}

	float const area = prop_->area(part_);

	bool const two_sided = material->is_two_sided();

	// this pdf includes the uv weight which adjusts for texture distortion by the shape
	if (!prop_->shape()->sample(part_, p, rs.uv, transformation, area, two_sided, result.shape)) {
		return false;
	}

	result.shape.pdf *= rs.pdf;
	result.radiance = material->sample_radiance(result.shape.wi, rs.uv, area,
												time, filter, worker);

	return true;
}

bool Prop_image_light::sample(f_float3 p, f_float3 n, float time,
							  Transformation const& transformation, bool total_sphere,
							  sampler::Sampler& sampler, uint32_t sampler_dimension,
							  Sampler_filter filter, Worker const& worker, Sample& result) const {
	auto const material = prop_->material(part_);

	float2 const s2d = sampler.generate_sample_2D(sampler_dimension);

	auto const rs = material->radiance_sample(s2d);
	if (0.f == rs.pdf) {
		return false;
	}

	float const area = prop_->area(part_);

	bool const two_sided = material->is_two_sided();

	// this pdf includes the uv weight which adjusts for texture distortion by the shape
	if (!prop_->shape()->sample(part_, p, rs.uv, transformation, area, two_sided, result.shape)) {
		return false;
	}

	if (math::dot(result.shape.wi, n) > 0.f || total_sphere) {
		result.shape.pdf *= rs.pdf;
		result.radiance = material->sample_radiance(result.shape.wi, rs.uv, area,
													time, filter, worker);

		return true;
	}

	return false;
}

float Prop_image_light::pdf(Ray const& ray, Intersection const& intersection, bool /*total_sphere*/,
							Sampler_filter filter, Worker const& worker) const {
	entity::Composed_transformation temp;
	auto const& transformation = prop_->transformation_at(ray.time, temp);

	float const area = prop_->area(part_);

	auto const material = prop_->material(part_);

	bool const two_sided = material->is_two_sided();

	// this pdf includes the uv weight which adjusts for texture distortion by the shape
	float const shape_pdf = prop_->shape()->pdf_uv(ray, intersection, transformation,
												   area, two_sided);

	float const material_pdf = material->emission_pdf(intersection.uv, filter, worker);

	return shape_pdf * material_pdf;
}

void Prop_image_light::prepare_sampling(uint32_t light_id, thread::Pool& pool) {
	prop_->prepare_sampling(part_, light_id, true, pool);
}

}
