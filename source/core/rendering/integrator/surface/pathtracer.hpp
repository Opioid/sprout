#ifndef SU_RENDERING_INTEGRATOR_SURFACE_PATHTRACER_HPP
#define SU_RENDERING_INTEGRATOR_SURFACE_PATHTRACER_HPP

#include "surface_integrator.hpp"
#include "sampler/sampler_golden_ratio.hpp"
#include "sampler/sampler_random.hpp"
#include "scene/scene_ray.hpp"

namespace scene::light { struct Sample; }

namespace rendering::integrator::surface {

class alignas(64) Pathtracer final : public Integrator {

public:

	struct Settings {
		uint32_t min_bounces;
		uint32_t max_bounces;
		float    path_continuation_probability;

		bool disable_caustics;
	};

	Pathtracer(rnd::Generator& rng, take::Settings const& take_settings,
			   const Settings& settings);

	virtual ~Pathtracer() override final;

	virtual void prepare(const Scene& scene, uint32_t num_samples_per_pixel) override final;

	virtual void resume_pixel(uint32_t sample, rnd::Generator& scramble) override final;

	virtual float3 li(Ray& ray, Intersection& intersection, Worker& worker) override final;

	virtual size_t num_bytes() const override final;

private:

	sampler::Sampler& material_sampler(uint32_t bounce);

	const Settings settings_;

	sampler::Random sampler_;

	static constexpr uint32_t Num_material_samplers = 3;
	sampler::Golden_ratio material_samplers_[Num_material_samplers];
};

class Pathtracer_factory final : public Factory {

public:

	Pathtracer_factory(take::Settings const& take_settings, uint32_t num_integrators,
					   uint32_t min_bounces, uint32_t max_bounces,
					   float path_termination_probability, bool enable_caustics);

	virtual ~Pathtracer_factory() override final;

	virtual Integrator* create(uint32_t id, rnd::Generator& rng) const override final;

private:

	Pathtracer* integrators_;

	Pathtracer::Settings settings_;
};

}

#endif
