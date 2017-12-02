#include "emission.hpp"
#include "rendering/rendering_worker.hpp"
#include "scene/scene.hpp"
#include "scene/scene_ray.inl"
#include "scene/shape/shape.hpp"
#include "scene/volume/volume.hpp"
#include "base/math/aabb.inl"
#include "base/math/vector3.inl"
#include "base/memory/align.hpp"

namespace rendering::integrator::volume {

Emission::Emission(rnd::Generator& rng, const take::Settings& take_settings,
				   const Settings& settings) :
	Integrator(rng, take_settings),
	settings_(settings) {}

void Emission::prepare(const scene::Scene& /*scene*/, uint32_t /*num_samples_per_pixel*/) {}

void Emission::resume_pixel(uint32_t /*sample*/, rnd::Generator& /*scramble*/) {}

float3 Emission::transmittance(const Ray& ray, const Volume& volume, const Worker& worker) {
	scene::entity::Composed_transformation temp;
	const auto& transformation = volume.transformation_at(ray.time, temp);

	const float3 tau = volume.optical_depth(transformation, ray, settings_.step_size, rng_,
											Sampler_filter::Nearest, worker);
	return math::exp(-tau);
}

float3 Emission::li(const Ray& ray, bool /*primary_ray*/, const Volume& volume,
					Worker& worker, float3& transmittance) {
	scene::entity::Composed_transformation temp;
	const auto& transformation = volume.transformation_at(ray.time, temp);

	const float3 emission = volume.emission(transformation, ray, settings_.step_size, rng_,
											Sampler_filter::Undefined, worker);

	transmittance = float3(1.f);

	return emission;
}

size_t Emission::num_bytes() const {
	return sizeof(*this);
}

Emission_factory::Emission_factory(const take::Settings& settings, uint32_t num_integrators,
								   float step_size) :
	Factory(settings, num_integrators),
	integrators_(memory::allocate_aligned<Emission>(num_integrators)),
	settings_{step_size} {}

Emission_factory::~Emission_factory() {
	memory::free_aligned(integrators_);
}

Integrator* Emission_factory::create(uint32_t id, rnd::Generator& rng) const {
	return new(&integrators_[id]) Emission(rng, take_settings_, settings_);
}

}
