#pragma once

#include "rendering/integrator/surface/surface_integrator.hpp"
#include "transmittance/transmittance_closed.hpp"
#include "sampler/sampler_ems.hpp"
#include "sampler/sampler_random.hpp"
#include "sampler/sampler_scrambled_hammersley.hpp"
#include "image/texture/sampler/sampler_2d_linear.hpp"
#include "image/texture/sampler/sampler_2d_nearest.hpp"
#include "image/texture/sampler/address_mode.hpp"

namespace scene {

namespace material { class Sample; }

}

namespace rendering { namespace integrator { namespace surface {

class Pathtracer_DL : public Integrator {
public:

	struct Settings {
		image::texture::sampler::Sampler_2D_linear <image::texture::sampler::Address_mode_repeat> sampler_linear;
		image::texture::sampler::Sampler_2D_nearest<image::texture::sampler::Address_mode_repeat> sampler_nearest;

		uint32_t min_bounces;
		uint32_t max_bounces;
		float    path_continuation_probability;

		uint32_t num_light_samples;
		float    num_light_samples_reciprocal;
		bool	 disable_caustics;
	};

	Pathtracer_DL(const take::Settings& take_settings, math::random::Generator& rng, const Settings& settings);

	virtual void start_new_pixel(uint32_t num_samples) final override;

	virtual math::float4 li(Worker& worker, scene::Ray& ray, bool volume,
							scene::Intersection& intersection) final override;

private:

	math::vec3 estimate_direct_light(Worker& worker, const scene::Ray& ray,
									   const scene::Intersection& intersection,
									   const scene::material::Sample& material_sample,
									   const image::texture::sampler::Sampler_2D& texture_sampler);

	const Settings& settings_;

	sampler::Random sampler_;

	transmittance::Closed transmittance_;
};

class Pathtracer_DL_factory : public Integrator_factory {
public:

	Pathtracer_DL_factory(const take::Settings& take_settings,
						  uint32_t min_bounces, uint32_t max_bounces,
						  float path_termination_probability,
						  uint32_t num_light_samples, bool disable_caustics);

	virtual Integrator* create(math::random::Generator& rng) const final override;

private:

	Pathtracer_DL::Settings settings_;
};

}}}
