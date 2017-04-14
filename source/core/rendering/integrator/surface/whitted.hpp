#pragma once

#include "surface_integrator.hpp"
#include "sampler/sampler_golden_ratio.hpp"
#include "sampler/sampler_random.hpp"

namespace scene { namespace material { class Sample; } }

namespace rendering { namespace integrator { namespace surface {

class Whitted : public Integrator {

public:

	struct Settings {
		uint32_t num_light_samples;
		float    num_light_samples_reciprocal;
	};

	Whitted(rnd::Generator& rng, const take::Settings& take_settings, const Settings& settings);

	virtual void prepare(const Scene& scene, uint32_t num_samples_per_pixel) final override;

	virtual void resume_pixel(uint32_t sample, rnd::Generator& scramble) final override;

	virtual float4 li(Worker& worker, Ray& ray, Intersection& intersection) final override;

	virtual size_t num_bytes() const final override;

private:

	float3 shade(Worker& worker, const Ray& ray, const Intersection& intersection);

	float3 estimate_direct_light(Worker& worker, const Ray& ray, const Intersection& intersection,
								 const Material_sample& material_sample);

	Settings settings_;

	sampler::Random sampler_;
};

class Whitted_factory : public Factory {

public:

	Whitted_factory(const take::Settings& take_settings, uint32_t num_light_samples);

	virtual Integrator* create(rnd::Generator& rng) const;

private:

	Whitted::Settings settings_;
};

}}}
