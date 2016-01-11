#pragma once

#include "rendering/integrator/volume/volume_integrator.hpp"

namespace rendering { namespace integrator { namespace volume {

class Emission : public Integrator {
public:

	Emission(const take::Settings& take_settings, math::random::Generator& rng);
};

class Emission_factory : public Integrator_factory {
public:

	Emission_factory(const take::Settings& settings);

	virtual Integrator* create(math::random::Generator& rng) const;
};

}}}
