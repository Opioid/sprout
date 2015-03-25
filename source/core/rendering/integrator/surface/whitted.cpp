#include "whitted.hpp"
#include "rendering/worker.hpp"
#include "scene/prop/prop_intersection.hpp"
#include "base/math/sampling.hpp"
#include "base/math/vector.inl"
#include "base/math/ray.inl"
#include "base/math/random/generator.inl"

namespace rendering {

Whitted::Whitted(uint32_t id, math::random::Generator& rng, const Settings& settings) : Surface_integrator(id, rng), settings_(settings) {}

math::float3 Whitted::li(const Worker& worker, uint32_t subsample, math::Oray& ray, scene::Intersection& intersection) {
	return math::float3(1.f, 0.f, 0.f);
}

Whitted_factory::Whitted_factory() {

}

Surface_integrator* Whitted_factory::create(uint32_t id, math::random::Generator& rng) const {
	return new Whitted(id, rng, settings_);
}

}

