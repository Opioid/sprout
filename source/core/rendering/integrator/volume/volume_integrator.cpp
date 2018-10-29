#include "volume_integrator.hpp"

namespace rendering::integrator::volume {

Integrator::Integrator(rnd::Generator& rng, take::Settings const& settings) noexcept
    : integrator::Integrator(rng, settings) {}

Integrator::~Integrator() noexcept {}

Factory::Factory(take::Settings const& settings, uint32_t num_integrators) noexcept
    : take_settings_(settings), num_integrators_(num_integrators) {}

Factory::~Factory() noexcept {}

}  // namespace rendering::integrator::volume
