#pragma once

#include "surface_integrator.hpp"
#include "transmittance/transmittance_closed.hpp"
#include "sampler/sampler_ems.hpp"
#include "sampler/sampler_random.hpp"
#include "sampler/sampler_hammersley.hpp"
#include "scene/material/sampler_settings.hpp"

namespace scene {

namespace material { class Sample; }

}

namespace rendering { namespace integrator { namespace surface {

class alignas(64) Pathtracer_DL : public Integrator {

public:

	struct Settings {
		uint32_t min_bounces;
		uint32_t max_bounces;
		float    path_continuation_probability;

		uint32_t num_light_samples;
		float    num_light_samples_reciprocal;
		bool	 enable_caustics;
	};

	Pathtracer_DL(rnd::Generator& rng, const take::Settings& take_settings,
				  const Settings& settings);

	virtual void prepare(const Scene& scene, uint32_t num_samples_per_pixel) final override;

	virtual void resume_pixel(uint32_t sample, rnd::Generator& scramble) final override;

	virtual float4 li(Worker& worker, Ray& ray, Intersection& intersection) final override;

	virtual size_t num_bytes() const final override;

private:

	float3 estimate_direct_light(Worker& worker, const Ray& ray,
								 const Intersection& intersection,
								 const Material_sample& material_sample,
								 Sampler_filter filter);

	const Settings settings_;

	sampler::Random sampler_;

	transmittance::Closed transmittance_;
};

class Pathtracer_DL_factory : public Factory {

public:

	Pathtracer_DL_factory(const take::Settings& take_settings, uint32_t num_integrators,
						  uint32_t min_bounces, uint32_t max_bounces,
						  float path_termination_probability,
						  uint32_t num_light_samples, bool enable_caustics);

	~Pathtracer_DL_factory();

	virtual Integrator* create(uint32_t id, rnd::Generator& rng) const final override;

private:

	Pathtracer_DL* integrators_;

	Pathtracer_DL::Settings settings_;
};

}}}
