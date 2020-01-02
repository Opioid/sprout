#include "volume_integrator.hpp"

namespace rendering::integrator::volume {

Integrator::Integrator(rnd::Generator& rng) noexcept : integrator::Integrator(rng) {}

Integrator::~Integrator() noexcept {}

Pool::Pool(uint32_t num_integrators) noexcept : num_integrators_(num_integrators) {}

Pool::~Pool() noexcept {}

}  // namespace rendering::integrator::volume
