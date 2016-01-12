#pragma once

#include "rendering/integrator/volume/volume_integrator.hpp"
#include "sampler/random_sampler.hpp"
#include "image/texture/sampler/sampler_2d_nearest.hpp"
#include "image/texture/sampler/address_mode.hpp"

namespace rendering { namespace integrator { namespace volume {

class Single_scattering : public Integrator {
public:

	struct Settings {
		image::texture::sampler::Sampler_2D_nearest<image::texture::sampler::Address_mode_repeat> sampler_nearest;
	};

	Single_scattering(const take::Settings& take_settings, math::random::Generator& rng, const Settings& settings);

	virtual math::float3 transmittance(const scene::volume::Volume* volume, const math::Oray& ray) final override;

	virtual math::float3 li(Worker& worker, const scene::volume::Volume* volume, const math::Oray& ray,
							math::float3& transmittance) final override;

private:

	Settings settings_;

	sampler::Random sampler_;
};

class Single_scattering_factory : public Integrator_factory {
public:

	Single_scattering_factory(const take::Settings& settings);

	virtual Integrator* create(math::random::Generator& rng) const;

private:

	Single_scattering::Settings settings_;
};

}}}
