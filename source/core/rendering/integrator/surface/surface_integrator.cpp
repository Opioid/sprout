#include "surface_integrator.hpp"
#include "image/texture/sampler/sampler_2d.hpp"
#include "rendering/rendering_worker.hpp"
#include "scene/scene_ray.inl"
#include "scene/scene_intersection.inl"
#include "take/take_settings.hpp"
#include "base/math/vector.inl"
#include "base/math/random/generator.inl"

namespace rendering { namespace integrator { namespace surface {

Integrator::Integrator(const take::Settings& settings, math::random::Generator& rng) :
	integrator::Integrator(settings, rng) {}

Integrator::~Integrator() {}

bool Integrator::resolve_mask(Worker& worker, scene::Ray& ray,
							  scene::Intersection& intersection,
							  Sampler_filter filter) {
	float opacity = intersection.opacity(worker, ray.time, filter);

	while (opacity < 1.f) {
		if (opacity > 0.f && opacity > rng_.random_float()) {
			return true;
		}

		// We never change the ray origin and just slide along the segment instead.
		// This seems to be more robust than setting the new origin from the last intersection.
		// Possible indicator of imprecision issues in other parts of the code,
		// but this seems to work well enough.
		ray.min_t = ray.max_t;
		ray.max_t = take_settings_.ray_max_t;
		if (!worker.intersect(ray, intersection)) {
			return false;
		}

		opacity = intersection.opacity(worker, ray.time, filter);
	}

	return true;
}

bool Integrator::intersect_and_resolve_mask(Worker& worker, scene::Ray& ray,
											scene::Intersection& intersection,
											Sampler_filter filter) {
	if (!worker.intersect(ray, intersection)) {
		return false;
	}

	return resolve_mask(worker, ray, intersection, filter);
}

Factory::Factory(const take::Settings& settings) :
	take_settings_(settings) {}

}}}
