#include "volume_integrator.hpp"
#include "take/take_settings.hpp"
#include "base/math/vector3.inl"
#include "base/random/generator.inl"

namespace rendering { namespace integrator { namespace volume {

Integrator::Integrator(const take::Settings& settings, rnd::Generator& rng) :
	integrator::Integrator(settings, rng) {}

Integrator::~Integrator() {}

Factory::Factory(const take::Settings& settings) : take_settings_(settings) {}

}}}
