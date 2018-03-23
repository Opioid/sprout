#ifndef SU_CORE_RENDERING_INTEGRATOR_SURFACE_PATHTRACER_DL1
#define SU_CORE_RENDERING_INTEGRATOR_SURFACE_PATHTRACER_DL1

#include "surface_integrator.hpp"
#include "transmittance/transmittance_closed.hpp"
#include "sampler/sampler_ems.hpp"
#include "sampler/sampler_random.hpp"
#include "sampler/sampler_hammersley.hpp"
#include "scene/material/sampler_settings.hpp"
#include <memory>

namespace scene::material { class Sample; }

namespace rendering::integrator::surface {

namespace sub { class Integrator; class Factory; }

class alignas(64) Pathtracer_DL1 final : public Integrator {

public:

	struct Settings {
		uint32_t min_bounces;
		uint32_t max_bounces;
		float    path_continuation_probability;

		uint32_t num_light_samples;
		float    num_light_samples_reciprocal;
		bool	 disable_caustics;
	};

	Pathtracer_DL1(rnd::Generator& rng, const take::Settings& take_settings,
				  const Settings& settings, sub::Integrator& subsurface);

	virtual void prepare(const Scene& scene, uint32_t num_samples_per_pixel) override final;

	virtual void resume_pixel(uint32_t sample, rnd::Generator& scramble) override final;

	virtual float3 li(Ray& ray, Intersection& intersection, Worker& worker) override final;

	virtual size_t num_bytes() const override final;

private:

	float3 direct_light(const Ray& ray, const Intersection& intersection,
						const Material_sample& material_sample,
						Sampler_filter filter, Worker& worker);

	const Settings settings_;

	sampler::Random sampler_;

	sub::Integrator& subsurface_;

	transmittance::Closed transmittance_;
};

class Pathtracer_DL1_factory final : public Factory {

public:

	Pathtracer_DL1_factory(const take::Settings& take_settings, uint32_t num_integrators,
						  std::unique_ptr<sub::Factory> sub_factory,
						  uint32_t min_bounces, uint32_t max_bounces,
						  float path_termination_probability,
						  uint32_t num_light_samples, bool enable_caustics);

	~Pathtracer_DL1_factory();

	virtual Integrator* create(uint32_t id, rnd::Generator& rng) const override final;

private:

	std::unique_ptr<sub::Factory> sub_factory_;

	Pathtracer_DL1* integrators_;

	Pathtracer_DL1::Settings settings_;
};

}

#endif
