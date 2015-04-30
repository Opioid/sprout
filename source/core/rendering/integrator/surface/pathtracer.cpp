#include "pathtracer.hpp"
#include "rendering/worker.hpp"
#include "image/texture/sampler/sampler_2d_nearest.inl"
#include "scene/scene.hpp"
#include "scene/prop/prop_intersection.hpp"
#include "scene/surrounding/surrounding.hpp"
#include "scene/light/light.hpp"
#include "scene/light/light_sample.hpp"
#include "scene/material/material.hpp"
#include "scene/material/material_sample.hpp"
#include "take/take_settings.hpp"
#include "base/math/sampling.hpp"
#include "base/math/vector.inl"
#include "base/math/ray.inl"
#include "base/math/random/generator.inl"

namespace rendering {

Pathtracer::Pathtracer(const take::Settings& take_settings, math::random::Generator& rng, const Settings& settings) :
	Surface_integrator(take_settings, rng), settings_(settings), sampler_(1, rng) {}

void Pathtracer::start_new_pixel(uint32_t num_samples) {
	sampler_.restart(num_samples);
}

math::float3 Pathtracer::li(Worker& worker, uint32_t subsample, math::Oray& ray, scene::Intersection& intersection) {
	sampler_.start_iteration(subsample);

	scene::material::Sample::Result sample_result;

	math::float3 throughput = math::float3(1.f, 1.f, 1.f);
	math::float3 result = math::float3::identity;

	for (uint32_t i = 0; i < settings_.max_bounces; ++i) {
		auto& material = intersection.material();

		math::float3 wo = -ray.direction;
		auto& material_sample = material.sample(intersection.geo, wo, settings_.sampler, worker.id());

		result += throughput * material_sample.emission();

		material_sample.sample_evaluate(sampler_, sample_result);
		if (0.f == sample_result.pdf) {
			break;
		}

		throughput *= sample_result.reflection / sample_result.pdf;

		float ray_offset = take_settings_.ray_offset_modifier * intersection.geo.epsilon;

		ray.origin = intersection.geo.p;
		ray.set_direction(sample_result.wi);
		ray.min_t  = ray_offset;
		ray.max_t = 1000.f;
		++ray.depth;

		bool hit = worker.intersect(ray, intersection);
		if (!hit) {
			math::float3 r = worker.scene().surrounding()->sample(ray);
			result += throughput * r;
			break;
		}
	}

	return result;
}

Pathtracer_factory::Pathtracer_factory(const take::Settings& take_settings, uint32_t min_bounces, uint32_t max_bounces) :
	Surface_integrator_factory(take_settings) {
	settings_.min_bounces = min_bounces;
	settings_.max_bounces = max_bounces;
}

Surface_integrator* Pathtracer_factory::create(math::random::Generator& rng) const {
	return new Pathtracer(take_settings_, rng, settings_);
}

}


