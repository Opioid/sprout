#include "pathtracer_dl.hpp"
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
//#include <iostream>

namespace rendering {

Pathtracer_DL::Pathtracer_DL(const take::Settings& take_settings, math::random::Generator& rng, const Settings& settings) :
	Surface_integrator(take_settings, rng), settings_(settings), sampler_(rng, 1) {}

void Pathtracer_DL::start_new_pixel(uint32_t num_samples) {
	sampler_.restart(num_samples);
}

math::float3 Pathtracer_DL::li(Worker& worker, uint32_t subsample, math::Oray& ray, scene::Intersection& intersection) {
	sampler_.start_iteration(subsample);

	scene::material::Sample::Result sample_result;

	bool hit = true;
	math::float3 throughput = math::float3(1.f, 1.f, 1.f);
	math::float3 result = math::float3::identity;

	for (uint32_t i = 0; i < settings_.max_bounces; ++i) {
		if (!resolve_mask(worker, ray, intersection)) {
			hit = false;
			break;
		}

		math::float3 wo = -ray.direction;
		auto material = intersection.material();
		auto& material_sample = material->sample(intersection.geo, wo, settings_.sampler, worker.id());

		if (0 == i) {
			if (math::dot(intersection.geo.n, wo) > 0.f) {
				result += material_sample.emission();
			}
		}

		float ray_offset = take_settings_.ray_offset_modifier * intersection.geo.epsilon;
		ray.origin = intersection.geo.p;
		ray.min_t  = ray_offset;
		++ray.depth;

		float light_pdf;
		scene::light::Light* light = worker.scene().montecarlo_light(rng_.random_float(), light_pdf);
		if (light) {
			light->sample(intersection.geo.p, ray.time, 1, sampler_, light_samples_);

			auto& ls = light_samples_[0];
			if (ls.pdf > 0.f) {
				ray.set_direction(ls.l);
				ray.max_t = ls.t - ray_offset;

				if (worker.visibility(ray)) {
					result += (throughput * ls.energy * material_sample.evaluate(ls.l)) / (light_pdf * ls.pdf);
				}
			}
		}

		material_sample.sample_evaluate(sampler_, sample_result);
		if (0.f == sample_result.pdf) {
			break;
		}

		throughput *= sample_result.reflection / sample_result.pdf;

		ray.set_direction(sample_result.wi);
		ray.max_t = 1000.f;

		hit = worker.intersect(ray, intersection);
		if (!hit) {
			break;
		}
	}

	if (!hit) {
		math::float3 r = worker.scene().surrounding()->sample(ray);
		result += throughput * r;
	}

/*
	if (math::contains_nan(result) || math::contains_inf(result)) {
		std::cout << "nan/inf" << std::endl;
	}
*/
	return result;
}

bool Pathtracer_DL::resolve_mask(Worker& worker, math::Oray& ray, scene::Intersection& intersection) {
	auto material = intersection.material();

	while (material->opacity(intersection.geo.uv, settings_.sampler) < 1.f) {
		// We never change the ray origin, but just slide along the segment.
		// This seems to be more robust than making the last intersection the new origin.
		// Possible indicator of imprecision issues in other parts of the code, but this seems to work well enough.
		ray.min_t = ray.max_t;
		ray.max_t = 1000.f;
		if (!worker.intersect(ray, intersection)) {
			return false;
		}

		material = intersection.material();
	}

	return true;
}

Pathtracer_DL_factory::Pathtracer_DL_factory(const take::Settings& take_settings, uint32_t min_bounces, uint32_t max_bounces) :
	Surface_integrator_factory(take_settings) {
	settings_.min_bounces = min_bounces;
	settings_.max_bounces = max_bounces;
}

Surface_integrator* Pathtracer_DL_factory::create(math::random::Generator& rng) const {
	return new Pathtracer_DL(take_settings_, rng, settings_);
}

}


