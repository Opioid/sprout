#pragma once

#include "rendering/integrator/volume/volume_integrator.hpp"
#include "sampler/sampler_random.hpp"
#include "image/texture/sampler/sampler_2d_nearest.hpp"
#include "image/texture/sampler/address_mode.hpp"

namespace rendering { namespace integrator { namespace volume {

class Single_scattering : public Integrator {
public:

	struct Settings {
		image::texture::sampler::Sampler_2D_nearest<image::texture::sampler::Address_mode_repeat> sampler_nearest;

		float step_size;
	};

	Single_scattering(const take::Settings& take_settings, math::random::Generator& rng, const Settings& settings);

	virtual math::vec3 transmittance(Worker& worker, const scene::volume::Volume* volume,
									   const scene::Ray& ray) final override;

	virtual math::float4 li(Worker& worker, const scene::volume::Volume* volume, const scene::Ray& ray,
							math::vec3& transmittance) final override;

private:

	const Settings& settings_;

	sampler::Random sampler_;
};

class Single_scattering_factory : public Integrator_factory {
public:

	Single_scattering_factory(const take::Settings& take_settings, float step_size);

	virtual Integrator* create(math::random::Generator& rng) const;

private:

	Single_scattering::Settings settings_;
};

}}}
