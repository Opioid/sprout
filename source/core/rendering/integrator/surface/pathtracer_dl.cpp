#include "pathtracer_dl.hpp"
#include "rendering/worker.hpp"
#include "scene/scene.hpp"
#include "scene/prop/prop_intersection.hpp"
#include "scene/surrounding/surrounding.hpp"
#include "scene/light/light.hpp"
#include "scene/light/light_sample.hpp"
#include "scene/material/material.hpp"
#include "scene/material/material_sample.hpp"
#include "base/math/sampling.hpp"
#include "base/math/vector.inl"
#include "base/math/ray.inl"
#include "base/math/random/generator.inl"

namespace rendering {

Pathtracer_DL::Pathtracer_DL(uint32_t id, math::random::Generator& rng, const Settings& settings) :
	Surface_integrator(id, rng), settings_(settings), sampler_(1, rng) {}

void Pathtracer_DL::start_new_pixel(uint32_t num_samples) {
	sampler_.restart(num_samples);
}

math::float3 Pathtracer_DL::li(const Worker& worker, uint32_t subsample, math::Oray& ray, scene::Intersection& intersection) {
	sampler_.start_iteration(subsample);

	math::float3 throughput = math::float3(1.f, 1.f, 1.f);
	math::float3 result = math::float3::identity;

	for (uint32_t i = 0; i < settings_.max_bounces; ++i) {
		uint32_t next_depth = ray.depth + 1;

		auto& material = intersection.material();
		// TODO: light material

		math::float3 wo = -ray.direction;
		auto& material_sample = material.sample(intersection.geo, wo, id_);

		ray.origin = intersection.geo.p;
		ray.min_t  = intersection.geo.epsilon;
		ray.depth  = next_depth;

		float light_pdf;
		scene::light::Light* light = worker.scene().montecarlo_light(rng_.random_float(), light_pdf);
		if (light) {
			light->sample(intersection.geo.p, ray.time, 1, sampler_, light_samples_);

			for (auto& ls : light_samples_) {
				if (ls.pdf > 0.f) {
					ray.set_direction(ls.l);
					ray.max_t = ls.t;

					if (worker.visibility(ray)) {
						result += (throughput * ls.energy * material_sample.evaluate(ls.l)) / (light_pdf * ls.pdf);
					}
				}
			}
		}

		math::float3 wi;
		float material_pdf;
		math::float3 r = material_sample.sample_evaluate(sampler_, wi, material_pdf);

		if (0.f == material_pdf) {
			break;
		}

		throughput *= r / material_pdf;

		ray.set_direction(wi);
		ray.max_t = 1000.f;

		bool hit = worker.intersect(ray, intersection);
		if (!hit) {
			r = worker.scene().surrounding()->sample(ray);
			result += throughput * r;
			break;
		}
	}

	/*
	shadow_ray.origin = intersection.geo.p;
	shadow_ray.min_t = intersection.geo.epsilon;
	shadow_ray.time = ray.time;

	auto& material = intersection.material();

	math::float3 wo = -ray.direction;
	auto& sample = material.sample(intersection.geo, wo, id_);

	for (auto l : worker.scene().lights()) {
		l->sample(intersection.geo.p, ray.time, 1, sampler_, light_samples_);

		for (auto& ls : light_samples_) {
			if (ls.pdf > 0.f) {
				shadow_ray.set_direction(ls.l);
				shadow_ray.max_t = ls.t;

				if (worker.visibility(shadow_ray)) {
					result += (ls.energy * sample.evaluate(ls.l)) / ls.pdf;
				}
			}
		}
	}
	*/
	return result;
}

Pathtracer_DL_factory::Pathtracer_DL_factory(uint32_t min_bounces, uint32_t max_bounces) {
	settings_.min_bounces = min_bounces;
	settings_.max_bounces = max_bounces;
}

Surface_integrator* Pathtracer_DL_factory::create(uint32_t id, math::random::Generator& rng) const {
	return new Pathtracer_DL(id, rng, settings_);
}

}


