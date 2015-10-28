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

void Prop_light::sample(const entity::Composed_transformation& transformation,
						const math::float3& p, const math::float3& n, bool total_sphere,
						const image::texture::sampler::Sampler_2D& image_sampler, sampler::Sampler& sampler,
						Sample& result) const {
	prop_->shape()->sample(part_, transformation, area_, p, n, total_sphere, sampler, result.shape);

	if (math::dot(result.shape.wi, n) > 0.f || total_sphere) {
		result.energy = prop_->material(part_)->sample_emission(result.shape.uv, image_sampler);
	} else {
		result.shape.pdf = 0.f;
	}
}

float Prop_light::pdf(const entity::Composed_transformation& transformation,
					  const math::float3& p, const math::float3& wi, bool total_sphere,
					  const image::texture::sampler::Sampler_2D& /*image_sampler*/,
					  shape::Node_stack& node_stack) const {
/*
	if (prop_->shape()->is_complex()) {
		math::Oray ray;
		ray.origin = p;
		ray.set_direction(wi);
		ray.min_t = 0.f;
		ray.max_t = 10000.f;
		if (!prop_->aabb().intersect_p(ray)) {
			return 0.f;
		}
	}
*/
	return prop_->shape()->pdf(part_, transformation, area_, p, wi, total_sphere, node_stack);
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
	prop_->shape()->prepare_sampling(part_, transformation.scale);
	area_ = prop_->shape()->area(part_, transformation.scale);
}

bool Prop_light::equals(const Prop* prop, uint32_t part) const {
	return prop_ == prop && part_ == part;
}

}}
