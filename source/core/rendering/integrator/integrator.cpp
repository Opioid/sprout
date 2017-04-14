#include "integrator.hpp"

namespace rendering { namespace integrator {

Integrator::Integrator(rnd::Generator& rng, const take::Settings& settings) :
	rng_(rng), take_settings_(settings) {}

Integrator::~Integrator() {}

const take::Settings& Integrator::take_settings() const {
	return take_settings_;
}

}}
