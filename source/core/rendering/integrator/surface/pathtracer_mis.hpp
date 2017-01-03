#pragma once

#include "rendering/integrator/surface/surface_integrator.hpp"
#include "transmittance/transmittance_closed.hpp"
#include "transmittance/transmittance_open.hpp"
#include "sampler/sampler_golden_ratio.hpp"
#include "sampler/sampler_random.hpp"

namespace scene {

namespace light { class Light; }
namespace material { class Sample; }

}

namespace rendering { namespace integrator { namespace surface {

class Pathtracer_MIS : public Integrator {

public:

	struct Settings {
		uint32_t min_bounces;
		uint32_t max_bounces;
		float    path_continuation_probability;

		Light_sampling light_sampling;

		float    num_light_samples_reciprocal;
		bool	 disable_caustics;
	};

	Pathtracer_MIS(const take::Settings& take_settings, rnd::Generator& rng,
				   const Settings& settings);

	virtual void prepare(const scene::Scene& scene, uint32_t num_samples_per_pixel) final override;

	virtual void resume_pixel(uint32_t sample, rnd::Generator& scramble) final override;

	virtual float4 li(Worker& worker, Ray& ray, Intersection& intersection) final override;

	virtual size_t num_bytes() const final override;

private:

	float3 estimate_direct_light(Worker& worker, const Ray& ray,
								 Intersection& intersection,
								 const scene::material::Sample& material_sample,
								 Sampler_filter filter,
								 Bxdf_result& sample_result);

	float3 evaluate_light(const scene::light::Light* light,
						  uint32_t sampler_dimension, float light_weight,
						  Worker& worker, Ray& ray,
						  const Intersection& intersection,
						  const scene::material::Sample& material_sample,
						  Sampler_filter filter);

	float3 resolve_transmission(Worker& worker, Ray& ray,
								Intersection& intersection,
								float3_p attenuation,
								Sampler_filter filter,
								Bxdf_result& sample_result);

	sampler::Sampler& material_sampler(uint32_t bounce);
	sampler::Sampler& light_sampler(uint32_t bounce);

	const Settings& settings_;

	sampler::Random sampler_;

	static constexpr uint32_t Num_material_samplers = 3;
	sampler::Golden_ratio material_samplers_[Num_material_samplers];

	static constexpr uint32_t Num_light_samplers = 3;
	sampler::Golden_ratio light_samplers_[Num_light_samplers];

	transmittance::Open   transmittance_open_;
	transmittance::Closed transmittance_closed_;
};

class Pathtracer_MIS_factory : public Factory {

public:

	Pathtracer_MIS_factory(const take::Settings& take_settings,
						   uint32_t min_bounces, uint32_t max_bounces,
						   float path_termination_probability,
						   Light_sampling light_sampling,
						   bool disable_caustics);

	virtual Integrator* create(rnd::Generator& rng) const final override;

private:

	Pathtracer_MIS::Settings settings_;
};

}}}



