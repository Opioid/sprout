#include "pathtracer.hpp"
#include "sub/sub_integrator.hpp"
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
					   const Settings& settings, sub::Integrator& subsurface) :
	Integrator(rng, take_settings),
	settings_(settings),
	sampler_(rng),
	material_samplers_{rng, rng, rng},
	subsurface_(subsurface),
	transmittance_(rng, take_settings) {}

Pathtracer::~Pathtracer() {
	memory::safe_destruct(subsurface_);
}

void Pathtracer::prepare(const Scene& scene, uint32_t num_samples_per_pixel) {
	sampler_.resize(num_samples_per_pixel, 1, 1, 1);

	for (auto& s : material_samplers_) {
		s.resize(num_samples_per_pixel, 1, 1, 1);
	}

	subsurface_.prepare(scene, num_samples_per_pixel);
}

void Pathtracer::resume_pixel(uint32_t sample, rnd::Generator& scramble) {
	sampler_.resume_pixel(sample, scramble);

	for (auto& s : material_samplers_) {
		s.resume_pixel(sample, scramble);
	}

	subsurface_.resume_pixel(sample, scramble);
}

float3 Pathtracer::li(Ray& ray, Intersection& intersection, Worker& worker) {
	Sampler_filter filter = ray.is_primary() ? Sampler_filter::Undefined
											 : Sampler_filter::Nearest;
	Bxdf_sample sample_result;

	float3 throughput(1.f);
	float3 result(0.f);
	float3 weight(1.f);

	// pathtracer needs as many iterations as bounces, because it has no forward prediction
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

		material_sample.sample(material_sampler(i, ray.properties), sample_result);
		if (0.f == sample_result.pdf) {
			break;
		}

//		const bool requires_bounce = sample_result.type.test_any(Bxdf_type::Specular,
//																 Bxdf_type::Transmission);

		const bool requires_bounce = sample_result.type.test(Bxdf_type::Specular);

		if (requires_bounce) {
			if (settings_.disable_caustics && !ray.is_primary()) {
				break;
			}
		} else {
			ray.set_primary(false);
			filter = Sampler_filter::Nearest;
		}

/*		if (sample_result.type.test(Bxdf_type::Transmission)) {
			if (material_sample.is_sss()) {
				result += throughput * subsurface_.li(ray, intersection, material_sample, 
													  Sampler_filter::Nearest,
													  worker, sample_result);
				if (0.f == sample_result.pdf) {
					break;
				}

				throughput *= sample_result.reflection;
			} else {
				transmittance_.resolve(ray, intersection, material_sample.absorption_coefficient(),
									   sampler_, Sampler_filter::Nearest, worker, sample_result);
				if (0.f == sample_result.pdf) {
					break;
				}

				throughput *= sample_result.reflection;
			}
		} else*/ {
			throughput *= sample_result.reflection / sample_result.pdf;
		}

		const float ray_offset = take_settings_.ray_offset_factor * intersection.geo.epsilon;
		ray.origin = intersection.geo.p;
		ray.set_direction(sample_result.wi);
		ray.min_t = ray_offset;
		ray.max_t = scene::Ray_max_t;
		++ray.depth;

//		float3 ssstr;
//		const bool hit = worker.intersect_and_resolve_mask(ray, intersection, material_sample,
//														   filter, ssstr);

//		float3 vtr;
//		const float3 vli = worker.volume_li(ray, vtr);
//		result += throughput * vli;
//		throughput *= ssstr * vtr;

//		if (!hit) {
//			break;
//		}




		const bool entering = sample_result.type.test(Bxdf_type::Transmission)
							&& !intersection.same_hemisphere(sample_result.wi);

		if (entering || intersection.geo.subsurface) {
			float3 vli;
			float3 vtr;
		//	float3 weight;
			const bool hit = worker.volume(ray, intersection, material_sample, vli, vtr, weight);

			result += throughput * vli;
			throughput *= vtr;

			if (!hit) {
				break;
			}
		} else {
			const bool hit = worker.intersect_and_resolve_mask(ray, intersection, filter);

			float3 vtr;
			const float3 vli = worker.volume_li(ray, vtr);
			result += throughput * vli;
			throughput *= vtr;

			weight = float3(1.f);

			if (!hit) {
				break;
			}
		}




	}

	return result;
}

sampler::Sampler& Pathtracer::material_sampler(uint32_t bounce, Ray::Properties properties) {
	if (Num_material_samplers > bounce && properties.test_not(Ray::Property::Recursive)) {
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
									   std::unique_ptr<sub::Factory> sub_factory,
									   uint32_t min_bounces, uint32_t max_bounces,
									   float path_termination_probability, bool enable_caustics) :
	Factory(take_settings),
	sub_factory_(std::move(sub_factory)),
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
	return new(&integrators_[id]) Pathtracer(rng, take_settings_, settings_,
											 *sub_factory_->create(id, rng));
}

}
