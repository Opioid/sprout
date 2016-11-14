#pragma once

#include "rendering/integrator/volume/volume_integrator.hpp"
#include "sampler/sampler_random.hpp"
#include "image/texture/sampler/sampler_2d_nearest.hpp"
#include "image/texture/sampler/address_mode.hpp"
#include "scene/material/sampler_settings.hpp"

namespace rendering { namespace integrator { namespace volume {

class Single_scattering : public Integrator {

public:

	struct Settings {
		float step_size;
	};

	Single_scattering(uint32_t num_samples_per_pixel,
					  const take::Settings& take_settings,
					  rnd::Generator& rng,
					  const Settings& settings);

	virtual void resume_pixel(uint32_t sample, rnd::Generator& scramble) final override;

	virtual float3 transmittance(Worker& worker, const scene::volume::Volume& volume,
								 const scene::Ray& ray) final override;

	virtual float4 li(Worker& worker, const scene::volume::Volume& volume,
					  const scene::Ray& ray, float3& transmittance) final override;

private:

	using Sampler_filter = scene::material::Sampler_settings::Filter;

	const Settings& settings_;

	sampler::Random sampler_;
};

class Single_scattering_factory : public Factory {

public:

	Single_scattering_factory(const take::Settings& take_settings, float step_size);

	virtual Integrator* create(uint32_t num_samples_per_pixel,
							   rnd::Generator& rng) const;

private:

	Single_scattering::Settings settings_;
};

}}}
