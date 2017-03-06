#include "attenuation.hpp"
#include "rendering/rendering_worker.hpp"
#include "scene/scene.hpp"
#include "scene/scene_ray.inl"
#include "scene/volume/volume.hpp"
#include "base/math/aabb.inl"
#include "base/math/ray.inl"
#include "base/math/vector3.inl"

namespace rendering { namespace integrator { namespace volume {

Attenuation::Attenuation(const take::Settings& take_settings,
						 rnd::Generator& rng) :
	Integrator(take_settings, rng) {}

void Attenuation::prepare(const scene::Scene& /*scene*/, uint32_t /*num_samples_per_pixel*/) {}

void Attenuation::resume_pixel(uint32_t /*sample*/, rnd::Generator& /*scramble*/) {}

float3 Attenuation::transmittance(Worker& worker, const Ray& ray, const Volume& volume) {
	float min_t;
	float max_t;
	if (!worker.scene().aabb().intersect_p(ray, min_t, max_t)) {
		return float3(1.f);
	}

	scene::Ray tray(ray.origin, ray.direction, min_t, max_t, ray.time);

	float3 tau = volume.optical_depth(tray, 1.f, rng_, worker, Sampler_filter::Nearest);
	return math::exp(-tau);
}

float4 Attenuation::li(Worker& worker, const Ray& ray, const Volume& volume,
					   float3& transmittance) {
	float min_t;
	float max_t;
	if (!worker.scene().aabb().intersect_p(ray, min_t, max_t)) {
		transmittance = float3(1.f);
		return math::float4_identity;
	}

	scene::Ray tray(ray.origin, ray.direction, min_t, max_t, ray.time);

	float3 tau = volume.optical_depth(tray, 1.f, rng_, worker, Sampler_filter::Unknown);
	transmittance = math::exp(-tau);

	return float4(0.f);
}

size_t Attenuation::num_bytes() const {
	return sizeof(*this);
}

Attenuation_factory::Attenuation_factory(const take::Settings& settings) :
	Factory(settings) {}

Integrator* Attenuation_factory::create(rnd::Generator& rng) const {
	return new Attenuation(take_settings_, rng);
}

}}}
