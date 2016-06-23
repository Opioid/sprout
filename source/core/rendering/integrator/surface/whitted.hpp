#pragma once

#include "surface_integrator.hpp"
#include "sampler/sampler_random.hpp"
#include "sampler/sampler_ems.hpp"
#include "image/texture/sampler/sampler_2d_linear.hpp"
#include "image/texture/sampler/sampler_2d_nearest.hpp"
#include "image/texture/sampler/address_mode.hpp"

namespace scene { namespace material { class Sample; } }

namespace rendering { namespace integrator { namespace surface {

class Whitted : public Integrator {

public:

	struct Settings {
		uint32_t num_light_samples;
		float    num_light_samples_reciprocal;
	};

	Whitted(const take::Settings& take_settings,
			math::random::Generator& rng, const Settings& settings);

	virtual void start_new_pixel(uint32_t num_samples);

	virtual float4 li(Worker& worker, scene::Ray& ray,
					  bool volume, scene::Intersection& intersection);

private:

	float3 shade(Worker& worker, const scene::Ray& ray,
				 const scene::Intersection& intersection);

	float3 estimate_direct_light(Worker& worker, const scene::Ray& ray,
								 const scene::Intersection& intersection,
								 const scene::material::Sample& material_sample);

	Settings settings_;

	sampler::Random sampler_;
};

class Whitted_factory : public Integrator_factory {

public:

	Whitted_factory(const take::Settings& take_settings, uint32_t num_light_samples);

	virtual Integrator* create(math::random::Generator& rng) const;

private:

	Whitted::Settings settings_;
};

}}}
