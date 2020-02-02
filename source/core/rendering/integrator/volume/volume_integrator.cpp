#include "volume_integrator.hpp"

namespace rendering::integrator::volume {

Integrator::Integrator(rnd::Generator& rng) : integrator::Integrator(rng) {}

Integrator::~Integrator() = default;

Pool::Pool(uint32_t num_integrators) : num_integrators_(num_integrators) {}

Pool::~Pool() = default;

}  // namespace rendering::integrator::volume
