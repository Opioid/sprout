#include "volume_integrator.hpp"
#include "rendering/integrator/integrator.inl"

namespace rendering::integrator {

namespace volume {

Integrator::Integrator() = default;

Integrator::~Integrator() = default;

}  // namespace volume

template class Pool<volume::Integrator>;

}  // namespace rendering::integrator
