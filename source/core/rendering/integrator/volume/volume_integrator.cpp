#include "volume_integrator.hpp"
#include "base/math/vector4.inl"
#include "base/random/generator.inl"

namespace rendering::integrator::volume {

Integrator::Integrator(rnd::Generator& rng, const take::Settings& settings) :
	integrator::Integrator(rng, settings) {}

Integrator::~Integrator() {}

Factory::Factory(const take::Settings& settings, uint32_t num_integrators) :
	take_settings_(settings), num_integrators_(num_integrators) {}

Factory::~Factory() {}

}
