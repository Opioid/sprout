#include "surface_integrator.hpp"
#include "image/texture/sampler/sampler_2d.hpp"
#include "rendering/rendering_worker.hpp"
#include "scene/scene_constants.hpp"
#include "scene/scene_intersection.inl"
#include "scene/scene_ray.inl"
#include "base/math/vector4.inl"
#include "base/random/generator.inl"

namespace rendering::integrator::surface {

Integrator::Integrator(rnd::Generator& rng, const take::Settings& settings) :
	integrator::Integrator(rng, settings) {}

Integrator::~Integrator() {}

bool Integrator::resolve_mask(Ray& ray, Intersection& intersection,
							  Sampler_filter filter, Worker& worker) {
	const float start_min_t = ray.min_t;

	float opacity = intersection.opacity(ray.time, filter, worker);

	while (opacity < 1.f) {
		if (opacity > 0.f && opacity > rng_.random_float()) {
			ray.min_t = start_min_t;
			return true;
		}

		// Slide along ray until opaque surface is found
		ray.min_t = ray.max_t + take_settings_.ray_offset_factor * intersection.geo.epsilon;
		ray.max_t = scene::Ray_max_t;
		if (!worker.intersect(ray, intersection)) {
			ray.min_t = start_min_t;
			return false;
		}

		opacity = intersection.opacity(ray.time, filter, worker);
	}

	ray.min_t = start_min_t;
	return true;
}

bool Integrator::intersect_and_resolve_mask(Ray& ray, Intersection& intersection,
											Sampler_filter filter, Worker& worker) {
	if (!worker.intersect(ray, intersection)) {
		return false;
	}

	return resolve_mask(ray, intersection, filter, worker);
}

Factory::Factory(const take::Settings& settings) :
	take_settings_(settings) {}

Factory::~Factory() {}

}
