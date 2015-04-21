#include "normal.hpp"
#include "rendering/worker.hpp"
#include "scene/prop/prop_intersection.hpp"
#include "base/math/sampling.hpp"
#include "base/math/vector.inl"
#include "base/math/ray.inl"
#include "base/math/random/generator.inl"

namespace rendering {

Normal::Normal(uint32_t id, math::random::Generator& rng, const Settings& settings) :
	Surface_integrator(id, rng), settings_(settings) {}

void Normal::start_new_pixel(uint32_t /*num_samples*/) {}

math::float3 Normal::li(const Worker& /*worker*/, uint32_t /*subsample*/, math::Oray& /*ray*/, scene::Intersection& intersection) {
	return 0.5f * (intersection.geo.b + math::float3(1.f, 1.f, 1.f));
}

Normal_factory::Normal_factory() {
}

Surface_integrator* Normal_factory::create(uint32_t id, math::random::Generator& rng) const {
	return new Normal(id, rng, settings_);
}

}
