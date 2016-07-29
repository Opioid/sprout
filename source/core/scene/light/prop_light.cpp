#include "prop_light.hpp"
#include "light_sample.hpp"
#include "scene/scene_worker.hpp"
#include "scene/prop.hpp"
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

const Light::Transformation& Prop_light::transformation_at(
		float time, Transformation& transformation) const {
	return prop_->transformation_at(time, transformation);
}

void Prop_light::sample(const Transformation& transformation, float time,
						float3_p p, float3_p n, bool total_sphere,
						sampler::Sampler& sampler, Worker& worker,
						Sampler_filter filter, Sample& result) const {
	auto material = prop_->material(part_);

	float area = prop_->area(part_);

	bool two_sided = material->is_two_sided();

	if (total_sphere) {
		prop_->shape()->sample(part_, transformation, p, area, two_sided,
							   sampler, worker.node_stack(), result.shape);
	} else {
		prop_->shape()->sample(part_, transformation, p, n, area, two_sided,
							   sampler, worker.node_stack(), result.shape);

		if (math::dot(result.shape.wi, n) <= 0.f) {
			result.shape.pdf = 0.f;
			return;
		}
	}

	result.radiance = material->sample_radiance(result.shape.wi, result.shape.uv,
												area, time, worker, filter);
}

float Prop_light::pdf(const Transformation& transformation,
					  float3_p p, float3_p wi, bool total_sphere,
					  Worker& worker, Sampler_filter /*filter*/) const {
	float area = prop_->area(part_);

	bool two_sided = prop_->material(part_)->is_two_sided();

	return prop_->shape()->pdf(part_, transformation, p, wi, area,
							   two_sided, total_sphere, worker.node_stack());
}

float3 Prop_light::power(const math::aabb& scene_bb) const {
	float area = prop_->area(part_);

	float3 radiance = prop_->material(part_)->average_radiance(area);

	if (prop_->shape()->is_finite()) {
		return area * radiance;
	} else {
		return math::squared_length(scene_bb.halfsize()) * area * radiance;
	}
}

void Prop_light::prepare_sampling() {
	prop_->material(part_)->prepare_sampling(false);

	prop_->prepare_sampling(part_);
}

bool Prop_light::equals(const Prop* prop, uint32_t part) const {
	return prop_ == prop && part_ == part;
}

}}
