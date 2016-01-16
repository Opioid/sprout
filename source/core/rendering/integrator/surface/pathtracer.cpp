#include "pathtracer.hpp"
#include "integrator_helper.hpp"
#include "rendering/rendering_worker.hpp"
#include "image/texture/sampler/sampler_2d_linear.inl"
#include "image/texture/sampler/sampler_2d_nearest.inl"
#include "scene/scene.hpp"
#include "scene/scene_ray.inl"
#include "scene/prop/prop_intersection.inl"
#include "scene/light/light.hpp"
#include "scene/light/light_sample.hpp"
#include "scene/material/bxdf.hpp"
#include "scene/material/material.hpp"
#include "scene/material/material_sample.hpp"
#include "take/take_settings.hpp"
#include "base/math/vector.inl"
#include "base/math/ray.inl"
#include "base/math/random/generator.inl"

namespace rendering { namespace integrator { namespace surface {

Pathtracer::Pathtracer(const take::Settings& take_settings, math::random::Generator& rng, const Settings& settings) :
	Integrator(take_settings, rng), settings_(settings), sampler_(rng, 1), transmittance_(take_settings, rng) {}

void Pathtracer::start_new_pixel(uint32_t num_samples) {
	sampler_.restart_and_seed(num_samples);
}

math::float4 Pathtracer::li(Worker& worker, scene::Ray& ray, bool volume, scene::Intersection& intersection) {
	scene::material::bxdf::Result sample_result;
	scene::material::bxdf::Result::Type previous_sample_type;

	math::float3 throughput = math::float3(1.f, 1.f, 1.f);
	math::float3 result = math::float3::identity;
	float opacity = 0.f;

	// pathtracer needs as many iterations as bounces, because it has no forward prediction
	for (uint32_t i = 0; i <= settings_.max_bounces; ++i) {
		bool primary_ray = 0 == i || previous_sample_type.test(scene::material::bxdf::Type::Specular);

		const image::texture::sampler::Sampler_2D* texture_sampler;

		if (primary_ray) {
			texture_sampler = &settings_.sampler_linear;
		} else {
			texture_sampler = &settings_.sampler_nearest;
		}

		if (!resolve_mask(worker, ray, intersection, *texture_sampler)) {
			break;
		}

		if (i > 0) {
		//	throughput *= worker.transmittance(ray);
			math::float3 tr;
			math::float4 vli = worker.volume_li(ray, tr);
			result += throughput * vli.xyz();
			throughput *= tr;
		}

		opacity = 1.f;

		math::float3 wo = -ray.direction;
		auto material = intersection.material();
		auto& material_sample = material->sample(intersection.geo, wo, 1.f, *texture_sampler, worker.id());

		if (material_sample.same_hemisphere(wo)) {
			result += throughput * material_sample.emission();
		}

		if (i == settings_.max_bounces) {
			break;
		}

		if (material_sample.is_pure_emissive()) {
			break;
		}

		material_sample.sample_evaluate(sampler_, sample_result);
		if (0.f == sample_result.pdf) {
			break;
		}

		if (ray.depth > 0 && settings_.disable_caustics
		&&  sample_result.type.test(scene::material::bxdf::Type::Specular)) {
			break;
		}

		if (sample_result.type.test(scene::material::bxdf::Type::Transmission)) {
			throughput *= transmittance_.resolve(worker, ray, intersection, material_sample.attenuation(),
												sampler_, settings_.sampler_nearest, sample_result);

			if (0.f == sample_result.pdf) {
				break;
			}
		} else {
			throughput *= sample_result.reflection / sample_result.pdf;
		}

		previous_sample_type = sample_result.type;

		float ray_offset = take_settings_.ray_offset_factor * intersection.geo.epsilon;
		ray.origin = intersection.geo.p;
		ray.set_direction(sample_result.wi);
		ray.min_t = ray_offset;
		ray.max_t = take_settings_.ray_max_t;
		++ray.depth;

		if (!worker.intersect(ray, intersection)) {
			break;
		}
	}

	return math::float4(result, opacity);
}

Pathtracer_factory::Pathtracer_factory(const take::Settings& take_settings,
									   uint32_t min_bounces, uint32_t max_bounces,
									   bool disable_caustics) :
	Integrator_factory(take_settings) {
	settings_.min_bounces = min_bounces;
	settings_.max_bounces = max_bounces;
	settings_.disable_caustics = disable_caustics;
}

Integrator* Pathtracer_factory::create(math::random::Generator& rng) const {
	return new Pathtracer(take_settings_, rng, settings_);
}

}}}
