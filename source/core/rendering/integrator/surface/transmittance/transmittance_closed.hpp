#pragma once

#include "rendering/integrator/integrator.hpp"
#include "base/math/vector.hpp"

namespace sampler { class Sampler; }

namespace rendering {

class Worker;

namespace integrator { namespace surface { namespace transmittance {

class Closed : public integrator::Integrator {

public:

	Closed(const take::Settings& take_settings, rnd::Generator& rng);

	virtual void prepare(const Scene& scene, uint32_t num_samples_per_pixel) final override;

	virtual void resume_pixel(uint32_t sample, rnd::Generator& scramble) final override;

	float3 resolve(Worker& worker, Ray& ray, Intersection& intersection,
				   float3_p attenuation, sampler::Sampler& sampler,
				   Sampler_filter filter, Bxdf_result& sample_result);

	virtual size_t num_bytes() const final override;
};

}}}}
