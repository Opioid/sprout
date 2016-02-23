#pragma once

#include "rendering/integrator/volume/volume_integrator.hpp"

namespace rendering { namespace integrator { namespace volume {

class Attenuation : public Integrator {
public:

	Attenuation(const take::Settings& take_settings, math::random::Generator& rng);

	virtual math::vec3 transmittance(Worker& worker, const scene::volume::Volume* volume,
									   const scene::Ray& ray) final override;

	virtual math::float4 li(Worker& worker, const scene::volume::Volume* volume, const scene::Ray& ray,
							math::vec3& transmittance) final override;
};

class Attenuation_factory : public Integrator_factory {
public:

	Attenuation_factory(const take::Settings& settings);

	virtual Integrator* create(math::random::Generator& rng) const;
};

}}}
