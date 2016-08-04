#include "attenuation.hpp"
#include "rendering/rendering_worker.hpp"
#include "scene/scene.hpp"
#include "scene/scene_ray.inl"
#include "scene/volume/volume.hpp"
#include "base/math/ray.inl"
#include "base/math/vector.inl"
#include "base/math/bounding/aabb.inl"

namespace rendering { namespace integrator { namespace volume {

Attenuation::Attenuation(const take::Settings& take_settings, math::random::Generator& rng) :
	Integrator(take_settings, rng) {}

float3 Attenuation::transmittance(Worker& worker,
								  const scene::volume::Volume* volume,
								  const scene::Ray& ray) {
	float min_t;
	float max_t;
	if (!worker.scene().aabb().intersect_p(ray, min_t, max_t)) {
		return float3(1.f, 1.f, 1.f);
	}

	scene::Ray tray(ray.origin, ray.direction, min_t, max_t, ray.time);

	float3 tau = volume->optical_depth(tray);
	return math::exp(-tau);
}

float4 Attenuation::li(Worker& worker, const scene::volume::Volume* volume,
					   const scene::Ray& ray, float3& transmittance) {
	float min_t;
	float max_t;
	if (!worker.scene().aabb().intersect_p(ray, min_t, max_t)) {
		transmittance = float3(1.f, 1.f, 1.f);
		return math::float4_identity;
	}

	scene::Ray tray(ray.origin, ray.direction, min_t, max_t, ray.time);

	float3 tau = volume->optical_depth(tray);
	transmittance = math::exp(-tau);

	return math::float4_identity;
}

Attenuation_factory::Attenuation_factory(const take::Settings& settings) :
	Factory(settings) {}

Integrator* Attenuation_factory::create(math::random::Generator& rng) const {
	return new Attenuation(take_settings_, rng);
}

}}}
