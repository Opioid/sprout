#pragma once

#include "volume_integrator.hpp"
#include "sampler/sampler_random.hpp"
#include "scene/material/sampler_settings.hpp"

namespace rendering { namespace integrator { namespace volume {

class Single_scattering : public Integrator {

public:

	struct Settings {
		float step_size;
	};

	Single_scattering(rnd::Generator& rng, const take::Settings& take_settings,
					  const Settings& settings);

	virtual void prepare(const Scene& scene, uint32_t num_samples_per_pixel) final override;

	virtual void resume_pixel(uint32_t sample, rnd::Generator& scramble) final override;

	virtual float3 transmittance(Worker& worker, const Ray& ray,
								 const Volume& volume) final override;

	virtual float4 li(Worker& worker, const Ray& ray, const Volume& volume,
					  float3& transmittance) final override;

	virtual size_t num_bytes() const final override;

private:

	const Settings& settings_;

	sampler::Random sampler_;
};

class Single_scattering_factory : public Factory {

public:

	Single_scattering_factory(const take::Settings& take_settings, float step_size);

	virtual Integrator* create(rnd::Generator& rng) const;

private:

	Single_scattering::Settings settings_;
};

}}}
