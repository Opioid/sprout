#ifndef SU_CORE_RENDERING_INTEGRATROR_SURFACE_SUB_BRUTEFORCE_HPP
#define SU_CORE_RENDERING_INTEGRATROR_SURFACE_SUB_BRUTEFORCE_HPP

#include "sub_integrator.hpp"
#include "sampler/sampler_random.hpp"

namespace scene {

namespace material { class BSSRDF; }

class Prop;

}

namespace rendering::integrator::surface::sub {

class alignas(64) Bruteforce final : public Integrator {

public:

	struct Settings {
		float step_size;
	};

	Bruteforce(rnd::Generator& rng, const take::Settings& take_settings, const Settings& settings);

	virtual void prepare(const Scene& scene, uint32_t num_samples_per_pixel) override final;

	virtual void resume_pixel(uint32_t sample, rnd::Generator& scramble) override final;

	virtual float3 li(const Ray& ray, bool primary_ray, Intersection& intersection,
					  const Material_sample& sample, Sampler_filter filter,
					  Worker& worker, Bxdf_sample& sample_result) override final;

	virtual size_t num_bytes() const override final;

private:

//	const Settings settings_;

	sampler::Random sampler_;
};

class Bruteforce_factory final : public Factory {

public:

	Bruteforce_factory(const take::Settings& take_settings, uint32_t num_integrators,
					   float step_size);

	~Bruteforce_factory();

	virtual Integrator* create(uint32_t id, rnd::Generator& rng) const override final;

private:

	Bruteforce* integrators_;

	Bruteforce::Settings settings_;
};

}

#endif
