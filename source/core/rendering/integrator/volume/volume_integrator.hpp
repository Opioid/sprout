#pragma once

#include "rendering/integrator/integrator.hpp"

namespace scene { namespace volume { class Volume; } }

namespace rendering { namespace integrator { namespace volume {

class Integrator : public integrator::Integrator {
public:

	Integrator(const take::Settings& settings, math::random::Generator& rng);
	virtual ~Integrator();

	virtual math::float3 transmittance(const scene::volume::Volume* volume, const math::Oray& ray) = 0;

	virtual math::float3 li(const scene::volume::Volume* volume, const math::Oray& ray) = 0;
};

class Integrator_factory {
public:

	Integrator_factory(const take::Settings& settings);

	virtual Integrator* create(math::random::Generator& rng) const = 0;

protected:

	const take::Settings& take_settings_;
};

}}}
