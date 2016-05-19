#include "prop_light.hpp"
#include "light_sample.hpp"
#include "scene/scene_worker.hpp"
#include "scene/prop/prop.hpp"
#include "scene/shape/shape.hpp"
#include "scene/shape/shape_sample.hpp"
#include "scene/material/material.hpp"
#include "base/math/vector.inl"
#include "base/math/matrix.inl"
#include "base/math/bounding/aabb.inl"

namespace scene { namespace light {

void Prop_light::init(Prop* prop, uint32_t part) {
	prop_ = prop;
	part_ = part;
}

const Light::Entity_transformation& Prop_light::transformation_at(
		float time, Entity_transformation& transformation) const {
	return prop_->transformation_at(time, transformation);
}

void Prop_light::sample(const Entity_transformation& transformation, float time,
						const math::float3& p, const math::float3& n, bool total_sphere,
						sampler::Sampler& sampler, Worker& worker,
						Sampler_filter filter, Sample& result) const {
	auto material = prop_->material(part_);

	bool two_sided = material->is_two_sided();

	if (total_sphere) {
		prop_->shape()->sample(part_, transformation, area_, p,
							   two_sided, sampler, worker.node_stack(), result.shape);
	} else {
		prop_->shape()->sample(part_, transformation, area_, p, n,
							   two_sided, sampler, worker.node_stack(), result.shape);

		if (math::dot(result.shape.wi, n) <= 0.f) {
			result.shape.pdf = 0.f;
			return;
		}
	}

	result.energy = material->sample_emission(result.shape.wi, result.shape.uv,
											  time, worker, filter);
}

float Prop_light::pdf(const Entity_transformation& transformation,
					  const math::float3& p, const math::float3& wi, bool total_sphere,
					  Worker& worker, Sampler_filter /*filter*/) const {
	bool two_sided = prop_->material(part_)->is_two_sided();

	return prop_->shape()->pdf(part_, transformation, area_, p, wi,
							   two_sided, total_sphere, worker.node_stack());
}

math::float3 Prop_light::power(const math::aabb& scene_bb) const {
	math::float3 emission = prop_->material(part_)->average_emission();

	if (prop_->shape()->is_finite()) {
		return area_ * emission;
	} else {
		return math::squared_length(scene_bb.halfsize()) * area_ * emission;
	}
}

void Prop_light::prepare_sampling() {
	prop_->material(part_)->prepare_sampling(false);

	prop_->shape()->prepare_sampling(part_);

	entity::Composed_transformation temp;
	auto& transformation = prop_->transformation_at(0.f, temp);
	area_ = prop_->shape()->area(part_, math::float3(transformation.scale));
}

bool Prop_light::equals(const Prop* prop, uint32_t part) const {
	return prop_ == prop && part_ == part;
}

}}
