#pragma once

#include "rendering/integrator/surface/surface_integrator.hpp"
#include "rendering/integrator/surface/sub/bruteforce.hpp"
#include "transmittance/transmittance_closed.hpp"
#include "sampler/sampler_ems.hpp"
#include "sampler/sampler_golden_ratio.hpp"
#include "sampler/sampler_random.hpp"

namespace scene { namespace light {

struct Sample;

}}

namespace rendering { namespace integrator { namespace surface {

class Pathtracer : public Integrator {

public:

	struct Settings {
		uint32_t min_bounces;
		uint32_t max_bounces;
		float    path_continuation_probability;

		bool disable_caustics;
	};

	Pathtracer(const take::Settings& take_settings, rnd::Generator& rng, const Settings& settings);

	virtual void prepare(const Scene& scene, uint32_t num_samples_per_pixel) final override;

	virtual void resume_pixel(uint32_t sample, rnd::Generator& scramble) final override;

	virtual float4 li(Worker& worker, scene::Ray& ray, Intersection& intersection) final override;

	virtual size_t num_bytes() const final override;

private:

	sampler::Sampler& material_sampler(uint32_t bounce);

	const Settings& settings_;

	sampler::Random sampler_;

	static constexpr uint32_t Num_material_samplers = 3;
	sampler::Golden_ratio material_samplers_[Num_material_samplers];

	transmittance::Closed transmittance_;

	sub::Bruteforce subsurface_;
};

class Pathtracer_factory : public Factory {

public:

	Pathtracer_factory(const take::Settings& take_settings,
					   uint32_t min_bounces, uint32_t max_bounces,
					   float path_termination_probability,
					   bool disable_caustics);

	virtual Integrator* create(rnd::Generator& rng) const final override;

private:

	Pathtracer::Settings settings_;
};

}}}
