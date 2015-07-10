#pragma once

#include "rendering/integrator/integrator.hpp"
#include "sampler/random_sampler.hpp"
#include "sampler/ems_sampler.hpp"
#include "image/texture/sampler/sampler_2d_nearest.hpp"
#include "image/texture/sampler/address_mode.hpp"
#include <vector>

namespace scene { namespace light {

struct Sample;

}}

namespace rendering {

class Whitted : public Surface_integrator {
public:

	struct Settings {
		uint32_t max_light_samples;

		image::texture::sampler::Sampler_2D_nearest<image::texture::sampler::Address_mode_repeat> sampler;
	};

	Whitted(const take::Settings& take_settings, math::random::Generator& rng, const Settings& settings);

	virtual void start_new_pixel(uint32_t num_samples);

	virtual math::float3 li(Worker& worker, math::Oray& ray, scene::Intersection& intersection);

private:

	math::float3 shade(Worker& worker, const math::Oray& ray, const scene::Intersection& intersection);

	Settings settings_;

	sampler::Random sampler_;

	std::vector<scene::light::Sample> light_samples_;
};

class Whitted_factory : public Surface_integrator_factory {
public:

	Whitted_factory(const take::Settings& take_settings, uint32_t max_light_samples);

	virtual Surface_integrator* create(math::random::Generator& rng) const;

private:

	Whitted::Settings settings_;
};

}

