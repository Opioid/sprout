#pragma once

#include "rendering/integrator/volume/volume_integrator.hpp"

namespace rendering { namespace integrator { namespace volume {

class Single_scattering : public Integrator {
public:

	Single_scattering(const take::Settings& take_settings, math::random::Generator& rng);

	virtual math::float3 transmittance(const scene::volume::Volume* volume, const math::Oray& ray) final override;

	virtual math::float3 li(const scene::volume::Volume* volume, const math::Oray& ray) final override;
};

class Single_scattering_factory : public Integrator_factory {
public:

	Single_scattering_factory(const take::Settings& settings);

	virtual Integrator* create(math::random::Generator& rng) const;
};

}}}
