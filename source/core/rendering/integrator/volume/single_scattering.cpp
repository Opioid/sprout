#include "single_scattering.hpp"
#include "scene/volume/volume.hpp"
#include "base/math/vector.inl"

namespace rendering { namespace integrator { namespace volume {

Single_scattering::Single_scattering(const take::Settings& take_settings, math::random::Generator& rng) :
	Integrator(take_settings, rng) {}

math::float3 Single_scattering::transmittance(const scene::volume::Volume* volume, const math::Oray& ray) {
	math::float3 tau = volume->optical_depth(ray);
	return math::exp(-tau);
}

math::float3 Single_scattering::li(const scene::volume::Volume* volume, const math::Oray& ray) {
	math::float3 tau = volume->optical_depth(ray);
	return math::exp(-tau);
}

Single_scattering_factory::Single_scattering_factory(const take::Settings& settings) :
	Integrator_factory(settings) {}

Integrator* Single_scattering_factory::create(math::random::Generator& rng) const {
	return new Single_scattering(take_settings_, rng);
}

}}}
