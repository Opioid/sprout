#include "surface_integrator.hpp"

namespace rendering::integrator::surface {

Integrator::Integrator(rnd::Generator& rng) noexcept : integrator::Integrator(rng) {}

Integrator::~Integrator() noexcept {}

Factory::Factory() noexcept {}

Factory::~Factory() noexcept {}

}  // namespace rendering::integrator::surface
