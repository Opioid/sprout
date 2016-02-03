#include "prop_light.hpp"
#include "light_sample.hpp"
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

void Prop_light::transformation_at(float time, entity::Composed_transformation& transformation) const {
	prop_->transformation_at(time, transformation);
}

void Prop_light::sample(const entity::Composed_transformation& transformation, float time,
						const math::float3& p, const math::float3& n, bool total_sphere,
						const image::texture::sampler::Sampler_2D& image_sampler,
						sampler::Sampler& sampler, shape::Node_stack& node_stack, Sample& result) const {
	auto material = prop_->material(part_);

	bool two_sided = material->is_two_sided();

	if (total_sphere) {
		prop_->shape()->sample(part_, transformation, area_, p, two_sided, sampler, node_stack, result.shape);
	} else {
		prop_->shape()->sample(part_, transformation, area_, p, n, two_sided, sampler, node_stack, result.shape);

		if (math::dot(result.shape.wi, n) <= 0.f) {
			result.shape.pdf = 0.f;
			return;
		}
	}

	result.energy = material->sample_emission(result.shape.uv, time, image_sampler);
}

float Prop_light::pdf(const entity::Composed_transformation& transformation,
					  const math::float3& p, const math::float3& wi, bool total_sphere,
					  const image::texture::sampler::Sampler_2D& /*image_sampler*/,
					  shape::Node_stack& node_stack) const {
	bool two_sided = prop_->material(part_)->is_two_sided();

	return prop_->shape()->pdf(part_, transformation, area_, p, wi, two_sided, total_sphere, node_stack);
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

	entity::Composed_transformation transformation;
	prop_->transformation_at(0.f, transformation);
	prop_->shape()->prepare_sampling(part_);
	area_ = prop_->shape()->area(part_, transformation.scale);
}

bool Prop_light::equals(const Prop* prop, uint32_t part) const {
	return prop_ == prop && part_ == part;
}

}}
