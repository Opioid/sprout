#ifndef SU_CORE_RENDERING_INTEGRATOR_VOLUME_TRACKING_MULTI_HPP
#define SU_CORE_RENDERING_INTEGRATOR_VOLUME_TRACKING_MULTI_HPP

#include "volume_integrator.hpp"

namespace rendering::integrator::volume {

class alignas(64) Tracking_multi : public Integrator {

public:

	Tracking_multi(rnd::Generator& rng, const take::Settings& take_settings);

	virtual void prepare(const Scene& scene, uint32_t num_samples_per_pixel) override final;

	virtual void resume_pixel(uint32_t sample, rnd::Generator& scramble) override final;

	virtual float3 transmittance(const Ray& ray, Worker& worker) override final;

	virtual bool integrate(Ray& ray, Intersection& intersection,
						   Sampler_filter filter, Worker& worker,
						   float3& li, float3& transmittance) override final;

	virtual size_t num_bytes() const override final;

private:

};

class Tracking_multi_factory final : public Factory {

public:

	Tracking_multi_factory(const take::Settings& take_settings, uint32_t num_integrators);

	virtual ~Tracking_multi_factory() override final;

	virtual Integrator* create(uint32_t id, rnd::Generator& rng) const override final;

private:

	Tracking_multi* integrators_;
};

}

#endif
