#include "volume_integrator.hpp"
#include "rendering/rendering_worker.hpp"
#include "take/take_settings.hpp"
#include "base/math/vector.inl"
#include "base/math/random/generator.inl"

namespace rendering { namespace integrator { namespace volume {

Integrator::Integrator(uint32_t num_samples_per_pixel,
					   const take::Settings& settings,
					   math::random::Generator& rng) :
	integrator::Integrator(num_samples_per_pixel, settings, rng) {}

Integrator::~Integrator() {}

Factory::Factory(const take::Settings& settings) : take_settings_(settings) {}

}}}
