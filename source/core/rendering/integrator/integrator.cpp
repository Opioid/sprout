#include "integrator.hpp"

namespace rendering {

Integrator::Integrator(math::random::Generator& rng) : rng_(rng) {}

void Integrator::start_new_pixel(uint32_t /*num_samples*/) {}

Surface_integrator::Surface_integrator(math::random::Generator& rng) : Integrator(rng) {}

}
