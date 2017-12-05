#ifndef SU_CORE_RENDERING_INTEGRATOR_SURFACE_TRANSMITTANCE_OPEN_HPP
#define SU_CORE_RENDERING_INTEGRATOR_SURFACE_TRANSMITTANCE_OPEN_HPP

#include "rendering/integrator/integrator.hpp"
#include "base/math/vector3.hpp"

namespace sampler { class Sampler; }

namespace scene {

namespace material::bxdf { struct Result; }

struct Intersection;
struct Ray;

}

namespace rendering {

class Worker;

namespace integrator::surface::transmittance {

class Open final : public integrator::Integrator {

public:

	Open(rnd::Generator& rng, const take::Settings& take_settings, uint32_t max_bounces);

	virtual void prepare(const scene::Scene& scene, uint32_t num_samples_per_pixel) override final;

	virtual void resume_pixel(uint32_t sample, rnd::Generator& scramble) override final;

	float3 resolve(const Ray& ray, Intersection& intersection,
				   const float3& absorption_coeffecient, sampler::Sampler& sampler,
				   Sampler_filter filter, Worker& worker, Bxdf_sample& sample_result) const;

	virtual size_t num_bytes() const override final;

private:

	uint32_t max_bounces_;
};

}}

#endif
