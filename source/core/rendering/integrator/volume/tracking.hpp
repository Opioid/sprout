#ifndef SU_CORE_RENDERING_INTEGRATOR_VOLUME_TRACKING_HPP
#define SU_CORE_RENDERING_INTEGRATOR_VOLUME_TRACKING_HPP

#include "scene/material/sampler_settings.hpp"
#include "base/math/vector3.hpp"

namespace rnd { class Generator; }

namespace scene {

namespace entity { struct Composed_transformation; }

namespace prop { struct Intersection; }

struct Ray;

}

namespace rendering {

class Worker;

namespace integrator::volume {

class Tracking {

public:

	using Ray		     = scene::Ray;
	using Transformation = scene::entity::Composed_transformation;
	using Intersection   = scene::prop::Intersection;
	using Sampler_filter = scene::material::Sampler_settings::Filter;

	static float3 transmittance(Ray const& ray, rnd::Generator& rng, Worker& worker);

	// Completely arbitrary and biased cutoff limit in order to prevent some worst case things
	static uint32_t constexpr max_iterations_ = 1024 * 64;
};

}}

#endif

