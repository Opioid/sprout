#include "volume_integrator.hpp"

namespace rendering::integrator::volume {

Integrator::Integrator(rnd::Generator& rng) noexcept : integrator::Integrator(rng) {}

Integrator::~Integrator() noexcept {}

Factory::Factory() noexcept {}

Factory::~Factory() noexcept {}

}  // namespace rendering::integrator::volume
