#include "sub_bruteforce.hpp"
#include "rendering/rendering_worker.hpp"
#include "scene/scene.hpp"
#include "scene/scene_constants.hpp"
#include "scene/scene_intersection.inl"
#include "scene/scene_ray.inl"
#include "scene/light/light.hpp"
#include "scene/light/light_sample.hpp"
#include "scene/material/bssrdf.hpp"
#include "base/math/vector4.inl"
#include "base/math/sampling/sampling.hpp"
#include "base/memory/align.hpp"
#include "base/random/generator.inl"

#include <iostream>

namespace rendering { namespace integrator { namespace surface { namespace sub {

Bruteforce::Bruteforce(rnd::Generator& rng, const take::Settings& take_settings,
					   const Settings& settings) :
	Integrator(rng, take_settings),
	settings_(settings),
	sampler_(rng) {}

void Bruteforce::prepare(const Scene& /*scene*/, uint32_t /*num_samples_per_pixel*/) {}

void Bruteforce::resume_pixel(uint32_t /*sample*/, rnd::Generator& /*scramble*/) {}

float3 Bruteforce::li(Worker& /*worker*/, const Ray& /*ray*/, Intersection& /*intersection*/,
					  const Material_sample& /*sample*/, Sampler_filter /*filter*/,
					  Bxdf_result& /*sample_result*/) {
	return float3(0.f);
}

size_t Bruteforce::num_bytes() const {
	return sizeof(*this) + sampler_.num_bytes();
}

Bruteforce_factory::Bruteforce_factory(const take::Settings& take_settings,
									   uint32_t num_integrators, float step_size) :
	Factory(take_settings),
	integrators_(memory::allocate_aligned<Bruteforce>(num_integrators)) {
	settings_.step_size = step_size;
}

Bruteforce_factory::~Bruteforce_factory() {
	memory::free_aligned(integrators_);
}

Integrator* Bruteforce_factory::create(uint32_t id, rnd::Generator& rng) const {
	return new(&integrators_[id]) Bruteforce(rng, take_settings_, settings_);
}

}}}}
