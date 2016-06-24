#include "integrator.hpp"

namespace rendering { namespace integrator {

Integrator::Integrator(const take::Settings& settings, math::random::Generator& rng) :
	take_settings_(settings), rng_(rng) {}

Integrator::~Integrator() {}

void Integrator::start_new_pixel(uint32_t /*num_samples*/) {}

const take::Settings& Integrator::take_settings() const {
	return take_settings_;
}

}}
