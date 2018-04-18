#include "pathtracer.hpp"
#include "rendering/rendering_worker.hpp"
#include "rendering/integrator/integrator_helper.hpp"
#include "scene/scene.hpp"
#include "scene/scene_constants.hpp"
#include "scene/scene_ray.inl"
#include "scene/light/light.hpp"
#include "scene/light/light_sample.hpp"
#include "scene/material/bxdf.hpp"
#include "scene/material/material.hpp"
#include "scene/material/material_sample.inl"
#include "scene/prop/prop_intersection.inl"
#include "base/math/vector3.inl"
#include "base/memory/align.hpp"
#include "base/random/generator.inl"
#include "base/spectrum/rgb.hpp"

namespace rendering::integrator::surface {

Pathtracer::Pathtracer(rnd::Generator& rng, const take::Settings& take_settings,
					   const Settings& settings) :
	Integrator(rng, take_settings),
	settings_(settings),
	sampler_(rng),
	material_samplers_{rng, rng, rng} {}

Pathtracer::~Pathtracer() {}

void Pathtracer::prepare(const Scene& /*scene*/, uint32_t num_samples_per_pixel) {
	sampler_.resize(num_samples_per_pixel, 1, 1, 1);

	for (auto& s : material_samplers_) {
		s.resize(num_samples_per_pixel, 1, 1, 1);
	}
}

void Pathtracer::resume_pixel(uint32_t sample, rnd::Generator& scramble) {
	sampler_.resume_pixel(sample, scramble);

	for (auto& s : material_samplers_) {
		s.resume_pixel(sample, scramble);
	}
}

float3 Pathtracer::li(Ray& ray, Intersection& intersection, Worker& worker) {
	Sampler_filter filter = ray.is_primary() ? Sampler_filter::Undefined
											 : Sampler_filter::Nearest;
	Bxdf_sample sample_result;

	float3 throughput(1.f);
	float3 result(0.f);

	bool was_subsurface = false;

	for (uint32_t i = ray.depth;; ++i) {
		const float3 wo = -ray.direction;
		const auto& material_sample = intersection.sample(wo, ray, filter, sampler_, worker);

		if (material_sample.same_hemisphere(wo)) {
			result += throughput * material_sample.radiance();
		}

		if (i >= settings_.max_bounces) {
			break;
		}

		if (material_sample.is_pure_emissive()) {
			break;
		}

		if (i > settings_.min_bounces) {
			const float q = std::max(spectrum::luminance(throughput),
									 settings_.path_continuation_probability);
			if (rendering::russian_roulette(throughput, q, sampler_.generate_sample_1D())) {
				break;
			}
		}

		material_sample.sample(material_sampler(ray.depth), sample_result);
		if (0.f == sample_result.pdf) {
			break;
		}

		if (!was_subsurface) {
			const bool requires_bounce = sample_result.type.test_any(Bxdf_type::Specular,
																	 Bxdf_type::Transmission);
			if (requires_bounce) {
				if (settings_.disable_caustics
				&&  material_sample.ior_greater_one() && !ray.is_primary()) {
					break;
				}
			} else {
				ray.set_primary(false);
				filter = Sampler_filter::Nearest;
			}
		}

		was_subsurface = intersection.geo.subsurface;

		throughput *= sample_result.reflection / sample_result.pdf;

		const float ray_offset = take_settings_.ray_offset_factor * intersection.geo.epsilon;

		if (material_sample.ior_greater_one()) {
			ray.origin = intersection.geo.p;
			ray.set_direction(sample_result.wi);
			ray.min_t = ray_offset;
			ray.max_t = scene::Ray_max_t;
			++ray.depth;
		} else {
			ray.min_t = ray.max_t + ray_offset;
			ray.max_t = scene::Ray_max_t;
		}

		if (sample_result.type.test(Bxdf_type::Transmission)) {
			worker.interface_change(sample_result.wi, intersection);
		}

		if (!worker.interface_stack().empty()) {
			float3 vli;
			float3 vtr;
			float3 weight;
			const bool hit = worker.volume(ray, intersection, filter, vli, vtr, weight);

			result += throughput * vli;
			throughput *= vtr * weight;

			if (!hit) {
				break;
			}
		} else if (!worker.intersect_and_resolve_mask(ray, intersection, filter)) {
			break;
		}
	}

	return result;
}

sampler::Sampler& Pathtracer::material_sampler(uint32_t bounce) {
	if (Num_material_samplers > bounce) {
		return material_samplers_[bounce];
	}

	return sampler_;
}

size_t Pathtracer::num_bytes() const {
	size_t sampler_bytes = 0;

	for (auto& s : material_samplers_) {
		sampler_bytes += s.num_bytes();
	}

	return sizeof(*this) + sampler_.num_bytes() + sampler_bytes;
}

Pathtracer_factory::Pathtracer_factory(const take::Settings& take_settings,
									   uint32_t num_integrators,
									   uint32_t min_bounces, uint32_t max_bounces,
									   float path_termination_probability, bool enable_caustics) :
	Factory(take_settings),
	integrators_(memory::allocate_aligned<Pathtracer>(num_integrators)),
	settings_ {
		min_bounces,
		max_bounces,
		1.f - path_termination_probability,
		!enable_caustics
	} {}

Pathtracer_factory::~Pathtracer_factory() {
	memory::free_aligned(integrators_);
}

Integrator* Pathtracer_factory::create(uint32_t id, rnd::Generator& rng) const {
	return new(&integrators_[id]) Pathtracer(rng, take_settings_, settings_);
}

}
