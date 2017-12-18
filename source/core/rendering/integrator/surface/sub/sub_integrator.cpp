#include "sub_integrator.hpp"
#include "rendering/rendering_worker.hpp"
#include "scene/scene.hpp"
#include "scene/scene_constants.hpp"
#include "scene/scene_ray.inl"
#include "scene/material/bssrdf.hpp"
#include "scene/light/light.hpp"
#include "scene/light/light_sample.hpp"
#include "scene/prop/prop_intersection.hpp"
#include "base/math/sampling/sampling.hpp"
#include "base/random/generator.inl"

#include "base/debug/assert.hpp"

namespace rendering::integrator::surface::sub {

Integrator::Integrator(rnd::Generator& rng, const take::Settings& settings) :
	integrator::Integrator(rng, settings) {}

Integrator::~Integrator() {}

Factory::Factory(const take::Settings& settings) :
	take_settings_(settings) {}

Factory::~Factory() {}

}
