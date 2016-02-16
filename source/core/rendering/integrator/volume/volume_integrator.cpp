#include "volume_integrator.hpp"
#include "rendering/rendering_worker.hpp"
#include "take/take_settings.hpp"
#include "base/math/vector.inl"
#include "base/math/random/generator.inl"

namespace rendering { namespace integrator { namespace volume {

Integrator::Integrator(const take::Settings& settings, math::random::Generator& rng) :
	integrator::Integrator(settings, rng) {}

Integrator::~Integrator() {}

Integrator_factory::Integrator_factory(const take::Settings& settings) : take_settings_(settings) {}

}}}
