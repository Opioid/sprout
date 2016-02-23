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

math::vec3 Attenuation::transmittance(Worker& worker, const scene::volume::Volume* volume, const scene::Ray& ray) {
	float min_t;
	float max_t;
	if (!worker.scene().aabb().intersect_p(ray, min_t, max_t)) {
		return math::vec3(1.f, 1.f, 1.f);
	}

	scene::Ray tray(ray.origin, ray.direction, min_t, max_t, ray.time);

	math::vec3 tau = volume->optical_depth(tray);
	return math::exp(-tau);
}

math::float4 Attenuation::li(Worker& worker, const scene::volume::Volume* volume, const scene::Ray& ray,
							 math::vec3& transmittance) {
	float min_t;
	float max_t;
	if (!worker.scene().aabb().intersect_p(ray, min_t, max_t)) {
		transmittance = math::vec3(1.f, 1.f, 1.f);
		return math::float4_identity;
	}

	scene::Ray tray(ray.origin, ray.direction, min_t, max_t, ray.time);

	math::vec3 tau = volume->optical_depth(tray);
	transmittance = math::exp(-tau);

	return math::float4_identity;
}

Attenuation_factory::Attenuation_factory(const take::Settings& settings) :
	Integrator_factory(settings) {}

Integrator* Attenuation_factory::create(math::random::Generator& rng) const {
	return new Attenuation(take_settings_, rng);
}

}}}
