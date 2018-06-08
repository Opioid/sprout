#include "surface_integrator.hpp"

namespace rendering::integrator::surface {

Integrator::Integrator(rnd::Generator& rng, take::Settings const& settings)
    : integrator::Integrator(rng, settings) {}

Integrator::~Integrator() {}

Factory::Factory(take::Settings const& settings) : take_settings_(settings) {}

Factory::~Factory() {}

}  // namespace rendering::integrator::surface
