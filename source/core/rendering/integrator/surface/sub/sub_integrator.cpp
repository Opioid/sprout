#include "sub_integrator.hpp"
#include "rendering/rendering_worker.hpp"
#include "take/take_settings.hpp"
#include "base/math/vector.inl"
#include "base/random/generator.inl"

namespace rendering { namespace integrator { namespace surface { namespace sub {

Integrator::Integrator(const take::Settings& settings, rnd::Generator& rng) :
	integrator::Integrator(settings, rng) {}

Integrator::~Integrator() {}

}}}}

