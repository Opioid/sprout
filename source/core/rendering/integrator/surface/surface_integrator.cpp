#include "surface_integrator.hpp"

namespace rendering::integrator::surface {

Integrator::Integrator(rnd::Generator& rng, const take::Settings& settings) :
	integrator::Integrator(rng, settings) {}

Integrator::~Integrator() {}

Factory::Factory(const take::Settings& settings) :
	take_settings_(settings) {}

Factory::~Factory() {}

}
