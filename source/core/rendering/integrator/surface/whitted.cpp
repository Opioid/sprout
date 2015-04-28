#include "whitted.hpp"
#include "rendering/worker.hpp"
#include "image/texture/sampler/sampler_2d_nearest.inl"
#include "scene/scene.hpp"
#include "scene/prop/prop_intersection.hpp"
#include "scene/light/light.hpp"
#include "scene/light/light_sample.hpp"
#include "scene/material/material.hpp"
#include "scene/material/material_sample.hpp"
#include "base/math/sampling.hpp"
#include "base/math/vector.inl"
#include "base/math/ray.inl"
#include "base/math/random/generator.inl"

namespace rendering {

Whitted::Whitted(math::random::Generator& rng, const Settings& settings) :
	Surface_integrator(rng), settings_(settings), sampler_(1, rng) {}

void Whitted::start_new_pixel(uint32_t num_samples) {
	sampler_.restart(num_samples);
}

math::float3 Whitted::li(Worker& worker, uint32_t subsample, math::Oray& ray, scene::Intersection& intersection) {
	sampler_.start_iteration(subsample);

	math::float3 result = math::float3::identity;

	math::Oray shadow_ray;
	shadow_ray.origin = intersection.geo.p;
	shadow_ray.min_t = intersection.geo.epsilon;
	shadow_ray.time = ray.time;

	auto& material = intersection.material();

	math::float3 wo = -ray.direction;
	auto& sample = material.sample(intersection.geo, wo, settings_.sampler, worker.id());

	for (auto l : worker.scene().lights()) {
		l->sample(intersection.geo.p, ray.time, 1, sampler_, light_samples_);

		for (auto& ls : light_samples_) {
			if (ls.pdf > 0.f) {
				shadow_ray.set_direction(ls.l);
				shadow_ray.max_t = ls.t - intersection.geo.epsilon;

				if (worker.visibility(shadow_ray)) {
					result += (ls.energy * sample.evaluate(ls.l)) / ls.pdf;
				}
			}
		}
	}

	return result;
}

Whitted_factory::Whitted_factory() {

}

Surface_integrator* Whitted_factory::create(math::random::Generator& rng) const {
	return new Whitted(rng, settings_);
}

}

