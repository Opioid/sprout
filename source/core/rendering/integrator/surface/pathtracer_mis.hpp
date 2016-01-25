#pragma once

#include "rendering/integrator/surface/surface_integrator.hpp"
#include "transmittance/transmittance_closed.hpp"
#include "transmittance/transmittance_open.hpp"
#include "sampler/sampler_ems.hpp"
#include "sampler/sampler_random.hpp"
#include "image/texture/sampler/sampler_2d_linear.hpp"
#include "image/texture/sampler/sampler_2d_nearest.hpp"
#include "image/texture/sampler/address_mode.hpp"

namespace scene {

namespace material { class Sample; }

}

namespace rendering { namespace integrator { namespace surface {

class Pathtracer_MIS : public Integrator {
public:

	struct Settings {
		image::texture::sampler::Sampler_2D_linear <image::texture::sampler::Address_mode_repeat> sampler_linear;
		image::texture::sampler::Sampler_2D_nearest<image::texture::sampler::Address_mode_repeat> sampler_nearest;

		uint32_t min_bounces;
		uint32_t max_bounces;
		uint32_t num_light_samples;
		float    num_light_samples_reciprocal;
		bool	 disable_caustics;
	};

	Pathtracer_MIS(const take::Settings& take_settings, math::random::Generator& rng, const Settings& settings);

	virtual void start_new_pixel(uint32_t num_samples) final override;

	virtual math::float4 li(Worker& worker, scene::Ray& ray, bool volume,
							scene::Intersection& intersection) final override;

private:

	math::float3 estimate_direct_light(Worker& worker, const scene::Ray& ray,
									   const scene::Intersection& intersection,
									   const scene::material::Sample& material_sample,
									   const image::texture::sampler::Sampler_2D& texture_sampler);

	math::float3 resolve_transmission(Worker& worker, scene::Ray& ray,
									  scene::Intersection& intersection,
									  const math::float3& attenuation,
									  const image::texture::sampler::Sampler_2D& texture_sampler,
									  scene::material::bxdf::Result& sample_result);

	const Settings& settings_;

	sampler::Random sampler_;

	transmittance::Open   transmittance_open_;
	transmittance::Closed transmittance_closed_;
};

class Pathtracer_MIS_factory : public Integrator_factory {
public:

	Pathtracer_MIS_factory(const take::Settings& take_settings,
						   uint32_t min_bounces, uint32_t max_bounces,
						   uint32_t num_light_samples, bool disable_caustics);

	virtual Integrator* create(math::random::Generator& rng) const final override;

private:

	Pathtracer_MIS::Settings settings_;
};

}}}



