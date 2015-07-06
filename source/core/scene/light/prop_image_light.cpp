#include "prop_image_light.hpp"
#include "light_sample.hpp"
#include "image/texture/texture_2d.inl"
#include "sampler/sampler.hpp"
#include "scene/prop/prop.hpp"
#include "scene/shape/shape.hpp"
#include "scene/shape/shape_sample.hpp"
#include "scene/material/material.hpp"
#include "base/color/color.inl"
#include "base/math/math.hpp"
#include "base/math/sampling.hpp"
#include "base/math/distribution_2d.inl"
#include "base/math/vector.inl"
#include "base/math/matrix.inl"
#include "base/math/bounding/aabb.inl"

namespace scene { namespace light {

void Prop_image_light::sample(const entity::Composed_transformation& transformation, const math::float3& p, const math::float3& n,
							  const image::sampler::Sampler_2D& image_sampler, sampler::Sampler& sampler,
							  uint32_t max_samples, std::vector<Sample>& samples) const {

	float pdf;
	math::float2 uv = distribution_.sample_continuous(sampler.generate_sample_1D(), sampler.generate_sample_1D(), pdf);

	samples.clear();

	Sample light_sample;

	prop_->shape()->sample(part_, transformation, area_, p, uv, light_sample.shape);

	light_sample.shape.pdf = /*200000.f * */pdf / (4.f * math::Pi);

	if (math::dot(light_sample.shape.wi, n) < 0.f) {
		light_sample.shape.pdf = 0.f;
	} else {
		light_sample.energy = prop_->material(part_)->sample_emission(uv, image_sampler);
	}

	samples.push_back(light_sample);

/*
	samples.clear();

	Sample light_sample;

	for (uint32_t i = 0; i < max_samples; ++i) {
		prop_->shape()->sample(part_, transformation, area_, p, n, sampler, light_sample.shape);

		light_sample.energy = prop_->material(part_)->sample_emission(light_sample.shape.uv, image_sampler);

		samples.push_back(light_sample);
	}
*/
}

float Prop_image_light::pdf(const entity::Composed_transformation& transformation, const math::float3& p, const math::float3& wi) const {
	return prop_->shape()->pdf(part_, transformation, area_, p, wi);
}

void Prop_image_light::prepare_sampling() {
	auto emission = prop_->material(part_)->emission_map();
	if (!emission) {
		return;
	}

	std::vector<float> luminance;
	auto d = emission->dimensions();
	luminance.resize(d.x * d.y);

	auto image = emission->image();
	for (uint32_t i = 0, len = static_cast<uint32_t>(luminance.size()); i < len; ++i) {
		luminance[i] = color::luminance(image->at3(i));
	}

	distribution_.init(luminance.data(), d);

	entity::Composed_transformation transformation;
	prop_->transformation_at(0.f, transformation);
	area_ = prop_->shape()->area(part_, transformation.scale);
}

}}

