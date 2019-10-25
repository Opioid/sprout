#include "integrator.hpp"

namespace rendering::integrator {

Integrator::Integrator(rnd::Generator& rng) noexcept : rng_(rng) {}

Integrator::~Integrator() noexcept {}

}  // namespace rendering::integrator
