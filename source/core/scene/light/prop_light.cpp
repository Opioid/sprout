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
						const math::float3& p, const math::float3& n, bool restrict_to_hemisphere,
						const image::texture::sampler::Sampler_2D& image_sampler, sampler::Sampler& sampler,
						uint32_t max_samples, std::vector<Sample>& samples) const {
	samples.clear();

	Sample light_sample;

	for (uint32_t i = 0; i < max_samples; ++i) {
		prop_->shape()->sample(part_, transformation, area_, p, n, restrict_to_hemisphere, sampler, light_sample.shape);

		if (restrict_to_hemisphere && math::dot(light_sample.shape.wi, n) <= 0.f) {
			// maybe don't push this sample at all instead?
			light_sample.shape.pdf = 0.f;
		} else {
			light_sample.energy = prop_->material(part_)->sample_emission(light_sample.shape.uv, image_sampler);
		}

		samples.push_back(light_sample);
	}
}

math::float3 Prop_light::evaluate(const math::float3& /*wi*/) const {
	return math::float3::identity;
}

float Prop_light::pdf(const entity::Composed_transformation& transformation,
					  const math::float3& p, const math::float3& wi, bool restrict_to_hemisphere,
					  const image::texture::sampler::Sampler_2D& /*image_sampler*/) const {
	return prop_->shape()->pdf(part_, transformation, area_, p, wi, restrict_to_hemisphere);
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
