#include "emission.hpp"

namespace rendering { namespace integrator { namespace volume {

Emission::Emission(const take::Settings& take_settings, math::random::Generator& rng) :
	Integrator(take_settings, rng) {}

Emission_factory::Emission_factory(const take::Settings& settings) :
	Integrator_factory(settings) {}

Integrator* Emission_factory::create(math::random::Generator& rng) const {
	return new Emission(take_settings_, rng);
}

}}}
