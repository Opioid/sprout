#include "prop_light.hpp"
#include "light_sample.hpp"
#include "scene/scene_ray.hpp"
#include "scene/scene_worker.hpp"
#include "scene/prop.hpp"
#include "scene/shape/shape.hpp"
#include "scene/shape/shape_sample.hpp"
#include "scene/material/material.hpp"
#include "base/math/aabb.inl"
#include "base/math/vector3.inl"
#include "base/math/matrix4x4.inl"

namespace scene { namespace light {

void Prop_light::init(Prop* prop, uint32_t part) {
	prop_ = prop;
	part_ = part;
}

const Light::Transformation& Prop_light::transformation_at(
		float time, Transformation& transformation) const {
	return prop_->transformation_at(time, transformation);
}

void Prop_light::sample(const Transformation& transformation,
						const float3& p, const float3& n, float time, bool total_sphere,
						sampler::Sampler& sampler, uint32_t sampler_dimension,
						Sampler_filter filter, Worker& worker, Sample& result) const {
	auto material = prop_->material(part_);

	const float area = prop_->area(part_);

	const bool two_sided = material->is_two_sided();

	if (total_sphere) {
		prop_->shape()->sample(part_, transformation, p, area, two_sided,
							   sampler, sampler_dimension,
							   worker.node_stack(), result.shape);
	} else {
		prop_->shape()->sample(part_, transformation, p, n, area, two_sided,
							   sampler, sampler_dimension,
							   worker.node_stack(), result.shape);

		if (math::dot(result.shape.wi, n) <= 0.f) {
			result.shape.pdf = 0.f;
			return;
		}
	}

	result.radiance = material->sample_radiance(result.shape.wi, result.shape.uv,
												area, time, worker, filter);
}

float Prop_light::pdf(const Ray& ray, const Intersection& intersection, bool total_sphere,
					  Sampler_filter /*filter*/, Worker& /*worker*/) const {
	entity::Composed_transformation temp;
	const auto& transformation = prop_->transformation_at(ray.time, temp);

	const float area = prop_->area(part_);

	const bool two_sided = prop_->material(part_)->is_two_sided();

	return prop_->shape()->pdf(ray, intersection, transformation, area, two_sided, total_sphere);
}

float3 Prop_light::power(const math::AABB& scene_bb) const {
	const float area = prop_->area(part_);

	const float3 radiance = prop_->material(part_)->average_radiance(area);

	if (prop_->shape()->is_finite()) {
		return area * radiance;
	} else {
		return math::squared_length(scene_bb.halfsize()) * area * radiance;
	}
}

void Prop_light::prepare_sampling(uint32_t light_id, thread::Pool& pool) {
	prop_->prepare_sampling(part_, light_id, false, pool);
}

bool Prop_light::equals(const Prop* prop, uint32_t part) const {
	return prop_ == prop && part_ == part;
}

}}
