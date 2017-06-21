#include "surface_integrator.hpp"
#include "image/texture/sampler/sampler_2d.hpp"
#include "rendering/rendering_worker.hpp"
#include "scene/scene_constants.hpp"
#include "scene/scene_intersection.inl"
#include "scene/scene_ray.inl"
#include "base/math/vector4.inl"
#include "base/random/generator.inl"

namespace rendering { namespace integrator { namespace surface {

Integrator::Integrator(rnd::Generator& rng, const take::Settings& settings) :
	integrator::Integrator(rng, settings) {}

Integrator::~Integrator() {}

bool Integrator::resolve_mask(Worker& worker, Ray& ray, Intersection& intersection,
							  Sampler_filter filter) {
	float opacity = intersection.opacity(worker, ray.time, filter);

	while (opacity < 1.f) {
		if (opacity > 0.f && opacity > rng_.random_float()) {
			return true;
		}

		// Slide along ray until opaque surface is found
		ray.min_t = ray.max_t + take_settings_.ray_offset_factor * intersection.geo.epsilon;
		ray.max_t = scene::Ray_max_t;
		if (!worker.intersect(ray, intersection)) {
			return false;
		}

		opacity = intersection.opacity(worker, ray.time, filter);
	}

	return true;
}

bool Integrator::intersect_and_resolve_mask(Worker& worker, Ray& ray, Intersection& intersection,
											Sampler_filter filter) {
	if (!worker.intersect(ray, intersection)) {
		return false;
	}

	return resolve_mask(worker, ray, intersection, filter);
}

Factory::Factory(const take::Settings& settings) :
	take_settings_(settings) {}

Factory::~Factory() {}

}}}
