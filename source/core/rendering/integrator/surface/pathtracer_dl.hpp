#pragma once

#include "rendering/integrator/integrator.hpp"
#include "sampler/random_sampler.hpp"
#include "image/texture/sampler/sampler_2d_nearest.hpp"
#include "image/texture/sampler/address_mode.hpp"
#include <vector>

namespace scene { namespace light {

struct Sample;

}}

namespace rendering {

class Pathtracer_DL : public Surface_integrator {
public:

	struct Settings {
		uint32_t min_bounces;
		uint32_t max_bounces;

		image::sampler::Sampler_2D_nearest<image::sampler::Address_mode_repeat> sampler;
	};

	Pathtracer_DL(const take::Settings& take_settings, math::random::Generator& rng, const Settings& settings);

	virtual void start_new_pixel(uint32_t num_samples);

	virtual math::float3 li(Worker& worker, uint32_t subsample, math::Oray& ray, scene::Intersection& intersection);

private:

	bool resolve_mask(Worker& worker, math::Oray& ray, scene::Intersection& intersection);

	Settings settings_;

	sampler::Random sampler_;

	std::vector<scene::light::Sample> light_samples_;
};

class Pathtracer_DL_factory : public Surface_integrator_factory {
public:

	Pathtracer_DL_factory(const take::Settings &take_settings, uint32_t min_bounces, uint32_t max_bounces);

	virtual Surface_integrator* create(math::random::Generator& rng) const;

private:

	Pathtracer_DL::Settings settings_;
};

}


