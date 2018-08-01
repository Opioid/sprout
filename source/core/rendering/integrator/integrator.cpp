#include "integrator.hpp"

namespace rendering::integrator {

Integrator::Integrator(rnd::Generator& rng, take::Settings const& settings) noexcept
    : rng_(rng), take_settings_(settings) {}

Integrator::~Integrator() noexcept {}

}  // namespace rendering::integrator
