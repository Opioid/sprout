#pragma once

#include "rendering/integrator/surface/surface_integrator.hpp"
#include "transmittance/transmittance_closed.hpp"
#include "sampler/sampler_ems.hpp"
#include "sampler/sampler_random.hpp"
#include "sampler/sampler_hammersley.hpp"
#include "image/texture/sampler/sampler_2d_linear.hpp"
#include "image/texture/sampler/sampler_2d_nearest.hpp"
#include "image/texture/sampler/address_mode.hpp"
#include "scene/material/sampler_settings.hpp"

namespace scene {

namespace material { class Sample; }

}

namespace rendering { namespace integrator { namespace surface {

class Pathtracer_DL : public Integrator {

public:

	struct Settings {
		uint32_t min_bounces;
		uint32_t max_bounces;
		float    path_continuation_probability;

		uint32_t num_light_samples;
		float    num_light_samples_reciprocal;
		bool	 disable_caustics;
	};

	Pathtracer_DL(const take::Settings& take_settings, rnd::Generator& rng,
				  const Settings& settings);

	virtual void prepare(const Scene& scene, uint32_t num_samples_per_pixel) final override;

	virtual void resume_pixel(uint32_t sample, rnd::Generator& scramble) final override;

	virtual float4 li(Worker& worker, Ray& ray, Intersection& intersection) final override;

	virtual size_t num_bytes() const final override;

private:

	float3 estimate_direct_light(Worker& worker, const Ray& ray,
								 const Intersection& intersection,
								 const scene::material::Sample& material_sample,
								 Sampler_filter filter);

	const Settings& settings_;

	sampler::Random sampler_;

	transmittance::Closed transmittance_;
};

class Pathtracer_DL_factory : public Factory {

public:

	Pathtracer_DL_factory(const take::Settings& take_settings,
						  uint32_t min_bounces, uint32_t max_bounces,
						  float path_termination_probability,
						  uint32_t num_light_samples, bool disable_caustics);

	virtual Integrator* create(rnd::Generator& rng) const final override;

private:

	Pathtracer_DL::Settings settings_;
};

}}}
