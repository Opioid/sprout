#include "whitted.hpp"
#include "rendering/worker.hpp"
#include "scene/prop/prop_intersection.hpp"
#include "scene/material/material.hpp"
#include "scene/material/material_sample.hpp"
#include "base/math/sampling.hpp"
#include "base/math/vector.inl"
#include "base/math/ray.inl"
#include "base/math/random/generator.inl"

namespace rendering {

Whitted::Whitted(uint32_t id, math::random::Generator& rng, const Settings& settings) : Surface_integrator(id, rng), settings_(settings) {}

math::float3 Whitted::li(const Worker& worker, uint32_t subsample, math::Oray& ray, scene::Intersection& intersection) {
	auto& material = intersection.material();

	auto& sample = material.sample(intersection.geo, id_);

	math::float3 result = sample.evaluate(math::float3(0.f, 0.f, 0.f));

	return result;
}

Whitted_factory::Whitted_factory() {

}

Surface_integrator* Whitted_factory::create(uint32_t id, math::random::Generator& rng) const {
	return new Whitted(id, rng, settings_);
}

}

