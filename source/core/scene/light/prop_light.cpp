#include "prop_light.hpp"
#include "light_sample.hpp"
#include "scene/prop/prop.hpp"
#include "scene/shape/shape.hpp"
#include "scene/material/material.hpp"
#include "base/math/vector.inl"
#include "base/math/matrix.inl"

namespace scene { namespace light {

void Prop_light::init(Prop* prop) {
	prop_ = prop;
}

void Prop_light::sample(const math::float3& p, float time, uint32_t /*max_samples*/, sampler::Sampler& sampler,
						std::vector<Sample>& samples) const {
	Composed_transformation transformation;
	prop_->transformation_at(time, transformation);

	samples.clear();

	Sample sample;

	prop_->shape()->importance_sample(transformation, p, sampler, 0, sample.l, sample.t, sample.pdf);

	sample.energy = prop_->material(0).sample_emission();

	samples.push_back(sample);
}

}}
