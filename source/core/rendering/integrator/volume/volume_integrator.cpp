#include "volume_integrator.hpp"

namespace rendering::integrator::volume {

Integrator::Integrator(rnd::Generator& rng, take::Settings const& settings)
    : integrator::Integrator(rng, settings) {}

Integrator::~Integrator() {}

Factory::Factory(take::Settings const& settings, uint32_t num_integrators)
    : take_settings_(settings), num_integrators_(num_integrators) {}

Factory::~Factory() {}

}  // namespace rendering::integrator::volume
