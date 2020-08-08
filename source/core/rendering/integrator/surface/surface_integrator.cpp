#include "surface_integrator.hpp"

namespace rendering::integrator::surface {

Integrator::Integrator() = default;

Integrator::~Integrator() = default;

Pool::Pool(uint32_t num_integrators) : num_integrators_(num_integrators) {}

Pool::~Pool() = default;

}  // namespace rendering::integrator::surface
