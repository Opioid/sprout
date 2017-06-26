#pragma once

#include "rendering/integrator/integrator.hpp"
#include "base/math/vector3.hpp"

namespace sampler { class Sampler; }

namespace rendering {

class Worker;

namespace integrator { namespace surface { namespace transmittance {

class Closed : public integrator::Integrator {

public:

	Closed(rnd::Generator& rng, const take::Settings& take_settings);

	virtual void prepare(const Scene& scene, uint32_t num_samples_per_pixel) final override;

	virtual void resume_pixel(uint32_t sample, rnd::Generator& scramble) final override;

	float3 resolve(Worker& worker, const Ray& ray, Intersection& intersection,
				   const float3& attenuation, sampler::Sampler& sampler,
				   Sampler_filter filter, Bxdf_result& sample_result) const;

	virtual size_t num_bytes() const final override;
};

}}}}
