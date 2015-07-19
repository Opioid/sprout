#include "pathtracer.hpp"
#include "integrator_helper.hpp"
#include "rendering/worker.hpp"
#include "image/texture/sampler/sampler_2d_linear.inl"
#include "image/texture/sampler/sampler_2d_nearest.inl"
#include "scene/scene.hpp"
#include "scene/prop/prop_intersection.inl"
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
	Surface_integrator(take_settings, rng), settings_(settings), sampler_(rng, 1) {}

void Pathtracer::start_new_pixel(uint32_t num_samples) {
	sampler_.restart(num_samples);
}

math::float3 Pathtracer::li(Worker& worker, math::Oray& ray, scene::Intersection& intersection) {
	scene::material::BxDF_result sample_result;
	scene::material::BxDF_result::Type previous_sample_type;
	math::float3 previous_sample_attenuation = math::float3(1.f, 1.f, 1.f);

	bool hit = true;
	math::float3 throughput = math::float3(1.f, 1.f, 1.f);
	math::float3 result = math::float3::identity;

	for (uint32_t i = 0; i < settings_.max_bounces; ++i) {
		bool primary_ray = 0 == i || previous_sample_type.test(scene::material::BxDF_type::Specular);

		const image::texture::sampler::Sampler_2D* texture_sampler;

		if (primary_ray) {
			texture_sampler = &settings_.sampler_linear;
		} else {
			texture_sampler = &settings_.sampler_nearest;
		}

		if (!resolve_mask(worker, ray, intersection, *texture_sampler)) {
			hit = false;
			break;
		}

		math::float3 wo = -ray.direction;
		auto material = intersection.material();
		auto& material_sample = material->sample(intersection.geo, wo, *texture_sampler, worker.id());

		if (material_sample.same_hemisphere(wo)) {
			result += throughput * material_sample.emission();
		} else {
			throughput *= attenuation(ray.origin, intersection.geo.p, previous_sample_attenuation);
		}

		if (material_sample.is_pure_emissive()) {
			break;
		}

		material_sample.sample_evaluate(sampler_, sample_result);
		if (0.f == sample_result.pdf) {
			break;
		}

		throughput *= sample_result.reflection / sample_result.pdf;

		previous_sample_type = sample_result.type;
		previous_sample_attenuation = material_sample.attenuation();

		float ray_offset = take_settings_.ray_offset_modifier * intersection.geo.epsilon;

		ray.origin = intersection.geo.p;
		ray.set_direction(sample_result.wi);
		ray.min_t = ray_offset;
		ray.max_t = 1000.f;
		++ray.depth;

		hit = worker.intersect(ray, intersection);
		if (!hit) {
			break;
		}
	}

	return result;
}

bool Pathtracer::resolve_mask(Worker& worker, math::Oray& ray, scene::Intersection& intersection,
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
		ray.max_t = 1000.f;
		if (!worker.intersect(ray, intersection)) {
			return false;
		}

		opacity = intersection.opacity(texture_sampler);
	}

	return true;
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


