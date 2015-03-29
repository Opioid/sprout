#include "integrator.hpp"

namespace rendering {

Integrator::Integrator(uint32_t id, math::random::Generator& rng) : id_(id), rng_(rng) {}

void Integrator::start_new_pixel(uint32_t num_samples) {}

Surface_integrator::Surface_integrator(uint32_t id, math::random::Generator& rng) : Integrator(id, rng) {}

}
