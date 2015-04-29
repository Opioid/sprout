#include "integrator.hpp"

namespace rendering {

Integrator::Integrator(const take::Settings& settings, math::random::Generator& rng) : take_settings_(settings), rng_(rng) {}

void Integrator::start_new_pixel(uint32_t /*num_samples*/) {}

Surface_integrator::Surface_integrator(const take::Settings& settings, math::random::Generator& rng) : Integrator(settings, rng) {}

Surface_integrator_factory::Surface_integrator_factory(const take::Settings& settings) : take_settings_(settings) {}

}
