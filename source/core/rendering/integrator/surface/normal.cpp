#include "normal.hpp"
#include "rendering/rendering_worker.hpp"
#include "scene/prop/prop_intersection.inl"
#include "scene/material/material.hpp"
#include "scene/material/material_sample.hpp"
#include "base/math/vector.inl"
#include "base/math/ray.inl"
#include "base/math/random/generator.inl"

namespace rendering { namespace integrator { namespace surface {

Normal::Normal(const take::Settings& take_settings, math::random::Generator& rng, const Settings& settings) :
	Integrator(take_settings, rng), settings_(settings) {}

void Normal::start_new_pixel(uint32_t /*num_samples*/) {}

math::float4 Normal::li(Worker& worker, math::Oray& ray, scene::Intersection& intersection) {
	math::float3 vector;

	if (Settings::Vector::Tangent == settings_.vector) {
		vector = intersection.geo.t;
	} else if (Settings::Vector::Bitangent == settings_.vector) {
		vector = intersection.geo.b;
	} else if (Settings::Vector::Geometric_normal == settings_.vector) {
		vector = intersection.geo.geo_n;
	} else if (Settings::Vector::Shading_normal == settings_.vector) {
		auto material = intersection.material();

		math::float3 wo = -ray.direction;
		auto& material_sample = material->sample(intersection.geo, wo, settings_.sampler, worker.id());

		vector = material_sample.shading_normal();
	}

	return math::float4(0.5f * (vector + math::float3(1.f, 1.f, 1.f)), 1.f);
}

Normal_factory::Normal_factory(const take::Settings& take_settings, Normal::Settings::Vector vector) :
	Integrator_factory(take_settings) {
	settings_.vector = vector;
}

Integrator* Normal_factory::create(math::random::Generator& rng) const {
	return new Normal(take_settings_, rng, settings_);
}

}}}
