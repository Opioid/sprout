#ifndef SU_CORE_RENDERING_INTEGRATOR_VOLUME_SINGLE_SCATTERING_TRACKING_HPP
#define SU_CORE_RENDERING_INTEGRATOR_VOLUME_SINGLE_SCATTERING_TRACKING_HPP

#include "volume_integrator.hpp"
#include "sampler/sampler_random.hpp"

namespace rendering::integrator::volume {

class alignas(64) Single_scattering_tracking final : public Integrator {

public:

	Single_scattering_tracking(rnd::Generator& rng, const take::Settings& take_settings);

	virtual void prepare(const Scene& scene, uint32_t num_samples_per_pixel) override final;

	virtual void resume_pixel(uint32_t sample, rnd::Generator& scramble) override final;

	virtual float3 transmittance(const Ray& ray, const Worker& worker) override final;

	virtual bool integrate(Ray& ray, Intersection& intersection,
						   Sampler_filter filter, Worker& worker,
						   float3& li, float3& transmittance, float3& weight) override final;

	virtual size_t num_bytes() const override final;

private:

	float3 direct_light(const Ray& ray, const float3& position, Worker& worker);

	float3 direct_light(const Ray& ray, const float3& position,
						const Intersection& intersection, Worker& worker);

	sampler::Random sampler_;
};

class Single_scattering_tracking_factory final : public Factory {

public:

	Single_scattering_tracking_factory(const take::Settings& take_settings, uint32_t num_integrators);

	~Single_scattering_tracking_factory() override final;

	virtual Integrator* create(uint32_t id, rnd::Generator& rng) const override final;

private:

	Single_scattering_tracking* integrators_;
};

}

#endif
