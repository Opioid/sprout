#pragma once

#include "rendering/integrator/integrator.hpp"
#include "base/math/vector3.hpp"

namespace sampler { class Sampler; }

namespace scene {

namespace material { namespace bxdf { struct Result; } }

struct Intersection;
struct Ray;

}

namespace rendering {

class Worker;

namespace integrator { namespace surface { namespace transmittance {

class Open : public integrator::Integrator {

public:

	Open(rnd::Generator& rng, const take::Settings& take_settings, uint32_t max_bounces);

	virtual void prepare(const scene::Scene& scene, uint32_t num_samples_per_pixel) override final;

	virtual void resume_pixel(uint32_t sample, rnd::Generator& scramble) override final;

	float3 resolve(const Ray& ray, Intersection& intersection,
				   const float3& absorption_coffecient, sampler::Sampler& sampler,
				   Sampler_filter filter, Worker& worker, Bxdf_sample& sample_result) const;

	virtual size_t num_bytes() const override final;

private:

	uint32_t max_bounces_;
};

}}}}
