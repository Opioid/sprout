#include "normal.hpp"
#include "rendering/worker.hpp"
#include "scene/prop/prop_intersection.inl"
#include "scene/material/material.hpp"
#include "scene/material/material_sample.hpp"
#include "base/math/sampling.hpp"
#include "base/math/vector.inl"
#include "base/math/ray.inl"
#include "base/math/random/generator.inl"

namespace rendering {

Normal::Normal(const take::Settings& take_settings, math::random::Generator& rng, const Settings& settings) :
	Surface_integrator(take_settings, rng), settings_(settings) {}

void Normal::start_new_pixel(uint32_t /*num_samples*/) {}

math::float3 Normal::li(Worker& worker, uint32_t /*subsample*/, math::Oray& ray, scene::Intersection& intersection) {
	auto material = intersection.material();

	math::float3 wo = -ray.direction;
	auto& material_sample = material->sample(intersection.geo, wo, settings_.sampler, worker.id());

	return 0.5f * (material_sample.geometric_normal() + math::float3(1.f, 1.f, 1.f));
}

Normal_factory::Normal_factory(const take::Settings& take_settings) : Surface_integrator_factory(take_settings) {
}

Surface_integrator* Normal_factory::create(math::random::Generator& rng) const {
	return new Normal(take_settings_, rng, settings_);
}

}
