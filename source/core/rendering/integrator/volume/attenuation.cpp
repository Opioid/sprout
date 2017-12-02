#include "attenuation.hpp"
#include "rendering/rendering_worker.hpp"
#include "scene/scene.hpp"
#include "scene/scene_ray.inl"
#include "scene/shape/shape.hpp"
#include "scene/volume/volume.hpp"
#include "base/math/aabb.inl"
#include "base/math/vector3.inl"
#include "base/memory/align.hpp"

namespace rendering::integrator::volume {

Attenuation::Attenuation(rnd::Generator& rng, const take::Settings& take_settings) :
	Integrator(rng, take_settings) {}

void Attenuation::prepare(const scene::Scene& /*scene*/, uint32_t /*num_samples_per_pixel*/) {}

void Attenuation::resume_pixel(uint32_t /*sample*/, rnd::Generator& /*scramble*/) {}

float3 Attenuation::transmittance(const Ray& ray, const Volume& volume, const Worker& worker) {
	Transformation temp;
	const auto& transformation = volume.transformation_at(ray.time, temp);

	const float3 tau = volume.optical_depth(transformation, ray, 1.f, rng_,
											Sampler_filter::Nearest, worker);
	return math::exp(-tau);
}

float3 Attenuation::li(const Ray& ray, bool /*primary_ray*/, const Volume& volume,
					   Worker& worker, float3& transmittance) {
	Transformation temp;
	const auto& transformation = volume.transformation_at(ray.time, temp);

	const float3 tau = volume.optical_depth(transformation, ray, 1.f, rng_,
											Sampler_filter::Undefined, worker);
	transmittance = math::exp(-tau);

	return float3(0.f);
}

size_t Attenuation::num_bytes() const {
	return sizeof(*this);
}

Attenuation_factory::Attenuation_factory(const take::Settings& settings, uint32_t num_integrators) :
	Factory(settings, num_integrators),
	integrators_(memory::allocate_aligned<Attenuation>(num_integrators)) {}

Attenuation_factory::~Attenuation_factory() {
	memory::free_aligned(integrators_);
}

Integrator* Attenuation_factory::create(uint32_t id, rnd::Generator& rng) const {
	return new(&integrators_[id]) Attenuation(rng, take_settings_);
}

}
