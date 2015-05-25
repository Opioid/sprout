#include "prop_light.hpp"
#include "light_sample.hpp"
#include "scene/prop/prop.hpp"
#include "scene/shape/shape.hpp"
#include "scene/material/material.hpp"
#include "base/math/vector.inl"
#include "base/math/matrix.inl"
#include "base/math/bounding/aabb.inl"

namespace scene { namespace light {

void Prop_light::init(Prop* prop, uint32_t part) {
	prop_ = prop;
	part_ = part;
}

void Prop_light::transformation_at(float time, Composed_transformation& transformation) const {
	prop_->transformation_at(time, transformation);
}

void Prop_light::sample(const math::float3& p, const Composed_transformation& transformation, uint32_t /*max_samples*/, sampler::Sampler& sampler,
						std::vector<Sample>& samples) const {
	samples.clear();

	Sample sample;

	prop_->shape()->importance_sample(part_, transformation, area_, p, sampler, 0, sample.l, sample.t, sample.pdf);

	sample.energy = prop_->material(part_)->sample_emission();

	samples.push_back(sample);
}

float Prop_light::pdf(const math::float3& p, const math::float3& wi, const Composed_transformation& transformation) const {
	return 1.f;
}

math::float3 Prop_light::energy(const math::aabb& scene_bb) const {
	if (prop_->shape()->is_finite()) {
		return area_ * prop_->material(part_)->average_emission();
	} else {
		return scene_bb.volume() * prop_->material(part_)->average_emission();
	}
}

void Prop_light::prepare_sampling() {
	Composed_transformation transformation;
	prop_->transformation_at(0.f, transformation);
	prop_->shape()->prepare_sampling(part_, transformation.scale);
	area_ = prop_->shape()->area(part_, transformation.scale);
}

bool Prop_light::equals(const Prop* prop, uint32_t part) const {
	return prop_ == prop && part_ == part;
}

}}
