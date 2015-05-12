#include "prop_light.hpp"
#include "light_sample.hpp"
#include "scene/prop/prop.hpp"
#include "scene/shape/shape.hpp"
#include "scene/material/material.hpp"
#include "base/math/vector.inl"
#include "base/math/matrix.inl"

namespace scene { namespace light {

void Prop_light::init(Prop* prop, uint32_t part) {
	prop_ = prop;
	part_ = part;
}

void Prop_light::sample(const math::float3& p, float time, uint32_t /*max_samples*/, sampler::Sampler& sampler,
						std::vector<Sample>& samples) const {
	Composed_transformation transformation;
	prop_->transformation_at(time, transformation);

	samples.clear();

	Sample sample;

	prop_->shape()->importance_sample(part_, transformation, area_, p, sampler, 0, sample.l, sample.t, sample.pdf);

	sample.energy = prop_->material(part_)->sample_emission();

	samples.push_back(sample);
}

void Prop_light::prepare_sampling() {
	Composed_transformation transformation;
	prop_->transformation_at(0.f, transformation);
	prop_->shape()->prepare_sampling(part_, transformation.scale);
	area_ = prop_->shape()->area(part_, transformation.scale);
}

}}
