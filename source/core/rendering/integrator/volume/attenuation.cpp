#include "attenuation.hpp"
#include "scene/volume/volume.hpp"
#include "base/math/vector.inl"

namespace rendering { namespace integrator { namespace volume {

Attenuation::Attenuation(const take::Settings& take_settings, math::random::Generator& rng) :
	Integrator(take_settings, rng) {}

math::float3 Attenuation::transmittance(const scene::volume::Volume* volume, const math::Oray& ray) {
	math::float3 tau = volume->optical_depth(ray);
	return math::exp(-tau);
}

math::float3 Attenuation::li(Worker& /*worker*/, const scene::volume::Volume* volume, const math::Oray& ray,
							 math::float3& transmittance) {
	math::float3 tau = volume->optical_depth(ray);
	transmittance = math::exp(-tau);

	return math::float3::identity;
}

Attenuation_factory::Attenuation_factory(const take::Settings& settings) :
	Integrator_factory(settings) {}

Integrator* Attenuation_factory::create(math::random::Generator& rng) const {
	return new Attenuation(take_settings_, rng);
}

}}}
