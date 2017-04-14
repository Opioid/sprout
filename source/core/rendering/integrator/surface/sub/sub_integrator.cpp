#include "sub_integrator.hpp"
#include "rendering/rendering_worker.hpp"
#include "take/take_settings.hpp"
#include "base/math/vector4.inl"
#include "base/random/generator.inl"

namespace rendering { namespace integrator { namespace surface { namespace sub {

Integrator::Integrator(rnd::Generator& rng, const take::Settings& settings) :
	integrator::Integrator(rng, settings) {}

Integrator::~Integrator() {}

}}}}

