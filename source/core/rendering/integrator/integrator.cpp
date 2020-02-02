#include "integrator.hpp"

namespace rendering::integrator {

Integrator::Integrator(rnd::Generator& rng) : rng_(rng) {}

Integrator::~Integrator() = default;

}  // namespace rendering::integrator
