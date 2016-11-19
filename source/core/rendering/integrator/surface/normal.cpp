#include "normal.hpp"
#include "rendering/rendering_worker.hpp"
#include "scene/scene_intersection.inl"
#include "scene/scene_ray.inl"
#include "scene/material/material.hpp"
#include "scene/material/material_sample.inl"
#include "take/take_settings.hpp"
#include "base/math/vector.inl"
#include "base/math/ray.inl"
#include "base/random/generator.inl"

namespace rendering { namespace integrator { namespace surface {

Normal::Normal(const take::Settings& take_settings,
			   rnd::Generator& rng,
			   const Settings& settings) :
	Integrator(take_settings, rng),
	settings_(settings) {}

void Normal::prepare(const scene::Scene& /*scene*/, uint32_t /*num_samples_per_pixel*/) {}

void Normal::resume_pixel(uint32_t /*sample*/, rnd::Generator& /*scramble*/) {}

float4 Normal::li(Worker& worker, scene::Ray& ray, bool /*volume*/,
				  scene::Intersection& intersection) {
	float3 vector;

	if (Settings::Vector::Tangent == settings_.vector) {
		vector = intersection.geo.t;
	} else if (Settings::Vector::Bitangent == settings_.vector) {
		vector = intersection.geo.b;
	} else if (Settings::Vector::Geometric_normal == settings_.vector) {
		vector = intersection.geo.geo_n;
	} else if (Settings::Vector::Shading_normal == settings_.vector) {
		float3 wo = -ray.direction;
		auto& material_sample = intersection.sample(worker, wo, ray.time,
													Sampler_filter::Unknown);

		if (!material_sample.same_hemisphere(wo)) {
			return float4(0.f, 0.f, 0.f, 1.f);
		}

		vector = material_sample.base_layer().shading_normal();
	} else {
		return float4(0.f, 0.f, 0.f, 1.f);
	}

	return float4(0.5f * (vector + float3(1.f, 1.f, 1.f)), 1.f);
}

Normal_factory::Normal_factory(const take::Settings& take_settings,
							   Normal::Settings::Vector vector) :
	Factory(take_settings) {
	settings_.vector = vector;
}

Integrator* Normal_factory::create(rnd::Generator& rng) const {
	return new Normal(take_settings_, rng, settings_);
}

}}}
