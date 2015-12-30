#pragma once

#include "rendering/integrator/integrator.hpp"
#include "transmission.hpp"
#include "sampler/ems_sampler.hpp"
#include "sampler/random_sampler.hpp"
#include "sampler/scrambled_hammersley_sampler.hpp"
#include "image/texture/sampler/sampler_2d_linear.hpp"
#include "image/texture/sampler/sampler_2d_nearest.hpp"
#include "image/texture/sampler/address_mode.hpp"

namespace scene {

namespace material { class Sample; }

}

namespace rendering {

class Pathtracer_DL : public Surface_integrator {
public:

	struct Settings {
		uint32_t min_bounces;
		uint32_t max_bounces;
		uint32_t num_light_samples;
		float    num_light_samples_reciprocal;
		bool	 disable_caustics;

		image::texture::sampler::Sampler_2D_linear <image::texture::sampler::Address_mode_repeat> sampler_linear;
		image::texture::sampler::Sampler_2D_nearest<image::texture::sampler::Address_mode_repeat> sampler_nearest;
	};

	Pathtracer_DL(const take::Settings& take_settings, math::random::Generator& rng, const Settings& settings);

	virtual void start_new_pixel(uint32_t num_samples) final override;

	virtual math::float4 li(Worker& worker, math::Oray& ray, scene::Intersection& intersection) final override;

private:

	math::float3 estimate_direct_light(Worker& worker, const math::Oray& ray,
									   const scene::Intersection& intersection,
									   const scene::material::Sample& material_sample,
									   const image::texture::sampler::Sampler_2D& texture_sampler);

	Settings settings_;

	sampler::Random sampler_;

	Transmission transmission_;
};

class Pathtracer_DL_factory : public Surface_integrator_factory {
public:

	Pathtracer_DL_factory(const take::Settings& take_settings, uint32_t min_bounces, uint32_t max_bounces,
						  uint32_t num_light_samples, bool disable_caustics);

	virtual Surface_integrator* create(math::random::Generator& rng) const final override;

private:

	Pathtracer_DL::Settings settings_;
};

}


