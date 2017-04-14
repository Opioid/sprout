#include "volume_integrator.hpp"
#include "base/math/vector4.inl"
#include "base/random/generator.inl"

namespace rendering { namespace integrator { namespace volume {

Integrator::Integrator(rnd::Generator& rng, const take::Settings& settings) :
	integrator::Integrator(rng, settings) {}

Integrator::~Integrator() {}

Factory::Factory(const take::Settings& settings) : take_settings_(settings) {}

}}}
