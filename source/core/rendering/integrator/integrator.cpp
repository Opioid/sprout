#include "integrator.hpp"

namespace rendering { namespace integrator {

Integrator::Integrator(uint32_t /*samples_per_pixel*/,
					   const take::Settings& settings,
					   math::random::Generator& rng) :
	take_settings_(settings), rng_(rng) {}

Integrator::~Integrator() {}

const take::Settings& Integrator::take_settings() const {
	return take_settings_;
}

}}
