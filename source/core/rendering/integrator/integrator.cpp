#include "integrator.hpp"

namespace rendering::integrator {

Integrator::Integrator(rnd::Generator& rng, take::Settings const& settings)
    : rng_(rng), take_settings_(settings) {}

Integrator::~Integrator() {}

}  // namespace rendering::integrator
