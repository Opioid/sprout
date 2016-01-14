#include "attenuation.hpp"
#include "rendering/rendering_worker.hpp"
#include "scene/scene.hpp"
#include "scene/volume/volume.hpp"
#include "base/math/ray.inl"
#include "base/math/vector.inl"

namespace rendering { namespace integrator { namespace volume {

Attenuation::Attenuation(const take::Settings& take_settings, math::random::Generator& rng) :
	Integrator(take_settings, rng) {}

math::float3 Attenuation::transmittance(Worker& worker, const scene::volume::Volume* volume, const math::Oray& ray) {
	float min_t;
	float max_t;
	if (!worker.scene().aabb().intersect_p(ray, min_t, max_t)) {
		return math::float3(1.f, 1.f, 1.f);
	}

	math::Oray tray(ray.origin, ray.direction, min_t, max_t);

	math::float3 tau = volume->optical_depth(tray);
	return math::exp(-tau);
}

math::float3 Attenuation::li(Worker& worker, const scene::volume::Volume* volume, const math::Oray& ray,
							 math::float3& transmittance) {
	float min_t;
	float max_t;
	if (!worker.scene().aabb().intersect_p(ray, min_t, max_t)) {
		transmittance = math::float3(1.f, 1.f, 1.f);
		return math::float3::identity;
	}

	math::Oray tray(ray.origin, ray.direction, min_t, max_t);

	math::float3 tau = volume->optical_depth(tray);
	transmittance = math::exp(-tau);

	return math::float3::identity;
}

Attenuation_factory::Attenuation_factory(const take::Settings& settings) :
	Integrator_factory(settings) {}

Integrator* Attenuation_factory::create(math::random::Generator& rng) const {
	return new Attenuation(take_settings_, rng);
}

}}}
