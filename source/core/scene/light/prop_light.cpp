#include "prop_light.hpp"
#include "light_sample.hpp"
#include "scene/scene_ray.hpp"
#include "scene/scene_worker.hpp"
#include "scene/prop/prop.hpp"
#include "scene/shape/shape.hpp"
#include "scene/shape/shape_sample.hpp"
#include "scene/material/material.hpp"
#include "base/math/aabb.inl"
#include "base/math/vector3.inl"
#include "base/math/matrix4x4.inl"

namespace scene::light {

void Prop_light::init(Prop* prop, uint32_t part) {
	prop_ = prop;
	part_ = part;
}

const Light::Transformation& Prop_light::transformation_at(
		float time, Transformation& transformation) const {
	return prop_->transformation_at(time, transformation);
}

bool Prop_light::sample(f_float3 p, float time, Transformation const& transformation,
						sampler::Sampler& sampler, uint32_t sampler_dimension,
						Sampler_filter filter, Worker const& worker, Sample& result) const {
	auto material = prop_->material(part_);

	float const area = prop_->area(part_);

	bool const two_sided = material->is_two_sided();

	if (!prop_->shape()->sample(part_, p, transformation, area, two_sided, sampler,
								sampler_dimension, worker.node_stack(), result.shape)) {
		return false;
	}

	result.radiance = material->sample_radiance(result.shape.wi, result.shape.uv,
												area, time, filter, worker);

	return true;
}

bool Prop_light::sample(f_float3 p, f_float3 n,
						float time, Transformation const& transformation, bool total_sphere,
						sampler::Sampler& sampler, uint32_t sampler_dimension,
						Sampler_filter filter, Worker const& worker, Sample& result) const {
	auto material = prop_->material(part_);

	float const area = prop_->area(part_);

	bool const two_sided = material->is_two_sided();

	if (total_sphere) {
		if (!prop_->shape()->sample(part_, p, transformation, area, two_sided, sampler,
									sampler_dimension, worker.node_stack(), result.shape)) {
			return false;
		}
	} else {
		if (!prop_->shape()->sample(part_, p, n, transformation, area, two_sided, sampler,
									sampler_dimension, worker.node_stack(), result.shape)) {
			return false;
		}

		if (math::dot(result.shape.wi, n) <= 0.f) {
			return false;
		}
	}

	result.radiance = material->sample_radiance(result.shape.wi, result.shape.uv,
												area, time, filter, worker);

	return true;
}

float Prop_light::pdf(Ray const& ray, Intersection const& intersection, bool total_sphere,
					  Sampler_filter /*filter*/, Worker const& /*worker*/) const {
	entity::Composed_transformation temp;
	auto const& transformation = prop_->transformation_at(ray.time, temp);

	float const area = prop_->area(part_);

	bool const two_sided = prop_->material(part_)->is_two_sided();

	return prop_->shape()->pdf(ray, intersection, transformation, area, two_sided, total_sphere);
}

float3 Prop_light::power(math::AABB const& scene_bb) const {
	float const area = prop_->area(part_);

	float3 const radiance = prop_->material(part_)->average_radiance(area);

	if (prop_->shape()->is_finite()) {
		return area * radiance;
	} else {
		return math::squared_length(scene_bb.halfsize()) * area * radiance;
	}
}

void Prop_light::prepare_sampling(uint32_t light_id, thread::Pool& pool) {
	prop_->prepare_sampling(part_, light_id, false, pool);
}

bool Prop_light::equals(Prop const* prop, uint32_t part) const {
	return prop_ == prop && part_ == part;
}

}
