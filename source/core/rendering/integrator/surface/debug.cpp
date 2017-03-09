#include "debug.hpp"
#include "rendering/rendering_worker.hpp"
#include "scene/scene_intersection.inl"
#include "scene/scene_ray.hpp"
#include "scene/material/material.hpp"
#include "scene/material/material_sample.inl"
#include "take/take_settings.hpp"
#include "base/math/vector4.inl"
#include "base/random/generator.inl"

namespace rendering { namespace integrator { namespace surface {

Debug::Debug(const take::Settings& take_settings, rnd::Generator& rng, const Settings& settings) :
	Integrator(take_settings, rng),
	settings_(settings) {}

void Debug::prepare(const scene::Scene& /*scene*/, uint32_t /*num_samples_per_pixel*/) {}

void Debug::resume_pixel(uint32_t /*sample*/, rnd::Generator& /*scramble*/) {}

float4 Debug::li(Worker& worker, Ray& ray, Intersection& intersection) {
	float3 vector;

	switch (settings_.vector) {
	case Settings::Vector::Tangent:
		vector = intersection.geo.t;
		break;
	case Settings::Vector::Bitangent:
		vector = intersection.geo.b;
		break;
	case Settings::Vector::Geometric_normal:
		vector = intersection.geo.geo_n;
		break;
	case Settings::Vector::Shading_normal: {
		float3 wo = -ray.direction;
		auto& material_sample = intersection.sample(worker, wo, ray.time, Sampler_filter::Unknown);

		if (!material_sample.same_hemisphere(wo)) {
			return float4(0.f, 0.f, 0.f, 1.f);
		}

		vector = material_sample.base_layer().shading_normal();
	}
		break;
	case Settings::Vector::UV:
		vector = float3(0.5f * intersection.geo.uv, -1.f);
		break;
	default:
		return float4(0.f, 0.f, 0.f, 1.f);
		break;
	}

	return float4(0.5f * (vector + float3(1.f)), 1.f);
}

size_t Debug::num_bytes() const {
	return sizeof(*this);
}

Debug_factory::Debug_factory(const take::Settings& take_settings, Debug::Settings::Vector vector) :
	Factory(take_settings) {
	settings_.vector = vector;
}

Integrator* Debug_factory::create(rnd::Generator& rng) const {
	return new Debug(take_settings_, rng, settings_);
}

}}}
