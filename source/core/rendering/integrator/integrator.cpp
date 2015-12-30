#include "integrator.hpp"
#include "image/texture/sampler/sampler_2d.hpp"
#include "rendering/rendering_worker.hpp"
#include "scene/prop/prop_intersection.inl"
#include "take/take_settings.hpp"
#include "base/math/random/generator.inl"

namespace rendering {

Integrator::Integrator(const take::Settings& settings, math::random::Generator& rng) : take_settings_(settings), rng_(rng) {}

Integrator::~Integrator() {}

void Integrator::start_new_pixel(uint32_t /*num_samples*/) {}

const take::Settings& Integrator::take_settings() const {
	return take_settings_;
}

Surface_integrator::Surface_integrator(const take::Settings& settings, math::random::Generator& rng) : Integrator(settings, rng) {}

Surface_integrator::~Surface_integrator() {}

bool Surface_integrator::resolve_mask(Worker& worker, math::Oray& ray, scene::Intersection& intersection,
									  const image::texture::sampler::Sampler_2D& texture_sampler) {
	float opacity = intersection.opacity(texture_sampler);

	while (opacity < 1.f) {
		if (opacity > 0.f && opacity > rng_.random_float()) {
			return true;
		}

		// We never change the ray origin and just slide along the segment instead.
		// This seems to be more robust than setting the new origin from the last intersection.
		// Possible indicator of imprecision issues in other parts of the code, but this seems to work well enough.
		ray.min_t = ray.max_t;
		ray.max_t = take_settings_.ray_max_t;
		if (!worker.intersect(ray, intersection)) {
			return false;
		}

		opacity = intersection.opacity(texture_sampler);
	}

	return true;
}

Surface_integrator_factory::Surface_integrator_factory(const take::Settings& settings) : take_settings_(settings) {}

}
