#include "attenuation.hpp"

namespace rendering { namespace integrator { namespace volume {

Attenuation::Attenuation(const take::Settings& take_settings, math::random::Generator& rng) :
	Integrator(take_settings, rng) {}

Attenuation_factory::Attenuation_factory(const take::Settings& settings) :
	Integrator_factory(settings) {}

Integrator* Attenuation_factory::create(math::random::Generator& rng) const {
	return new Attenuation(take_settings_, rng);
}

}}}
