#include "surface_integrator.hpp"

namespace rendering::integrator::surface {

Integrator::Integrator(rnd::Generator& rng, take::Settings const& settings) noexcept
    : integrator::Integrator(rng, settings) {}

Integrator::~Integrator() noexcept {}

Factory::Factory(take::Settings const& settings) noexcept : take_settings_(settings) {}

Factory::~Factory() noexcept {}

}  // namespace rendering::integrator::surface
