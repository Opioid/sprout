#pragma once

#include "rendering/integrator/integrator.hpp"
#include "sampler/random_sampler.hpp"
#include "image/texture/sampler/sampler_2d_linear.hpp"
#include "image/texture/sampler/sampler_2d_nearest.hpp"
#include "image/texture/sampler/address_mode.hpp"
#include <vector>

namespace scene { namespace light {

struct Sample;

}}

namespace rendering {

class Pathtracer : public Surface_integrator {
public:

	struct Settings {
		uint32_t min_bounces;
		uint32_t max_bounces;

		image::texture::sampler::Sampler_2D_linear <image::texture::sampler::Address_mode_repeat> sampler_linear;
		image::texture::sampler::Sampler_2D_nearest<image::texture::sampler::Address_mode_repeat> sampler_nearest;
	};

	Pathtracer(const take::Settings& take_settings, math::random::Generator& rng, const Settings& settings);

	virtual void start_new_pixel(uint32_t num_samples) final override;

	virtual math::float3 li(Worker& worker, math::Oray& ray, scene::Intersection& intersection) final override;

private:

	bool resolve_mask(Worker& worker, math::Oray& ray, scene::Intersection& intersection,
					  const image::texture::sampler::Sampler_2D& texture_sampler);

	Settings settings_;

	sampler::Random sampler_;

	std::vector<scene::light::Sample> light_samples_;
};

class Pathtracer_factory : public Surface_integrator_factory {
public:

	Pathtracer_factory(const take::Settings &take_settings, uint32_t min_bounces, uint32_t max_bounces);

	virtual Surface_integrator* create(math::random::Generator& rng) const final override;

private:

	Pathtracer::Settings settings_;
};

}


