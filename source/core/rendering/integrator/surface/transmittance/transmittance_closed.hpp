#pragma once

#include "rendering/integrator/integrator.hpp"
#include "base/math/vector3.hpp"
// #include "sampler/sampler_golden_ratio.hpp"

namespace sampler { class Sampler; }

namespace rendering {

class Worker;

namespace integrator { namespace surface { namespace transmittance {

class Closed : public integrator::Integrator {

public:

	Closed(rnd::Generator& rng, const take::Settings& take_settings);

	virtual void prepare(const Scene& scene, uint32_t num_samples_per_pixel) override final;

	virtual void resume_pixel(uint32_t sample, rnd::Generator& scramble) override final;

	float3 resolve(const Ray& ray, Intersection& intersection,
				   const float3& attenuation, sampler::Sampler& sampler,
				   Sampler_filter filter, Worker& worker, Bxdf_sample& sample_result);

	virtual size_t num_bytes() const override final;

private:

//	sampler::Golden_ratio samplers_[2];
};

}}}}
