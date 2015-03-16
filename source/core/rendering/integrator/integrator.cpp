#include "integrator.hpp"

namespace rendering {

Integrator::Integrator(uint32_t id, const math::random::Generator& rng) : rng_(rng) {}

Surface_integrator::Surface_integrator(uint32_t id, const math::random::Generator& rng) : Integrator(id, rng) {}

}
