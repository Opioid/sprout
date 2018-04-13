#include "volume_integrator.hpp"
#include "base/math/vector3.inl"

namespace rendering::integrator::volume {

Integrator::Integrator(rnd::Generator& rng, const take::Settings& settings) :
	integrator::Integrator(rng, settings) {}

Integrator::~Integrator() {}

bool Integrator::integrate(Ray& /*ray*/, Intersection& /*intersection*/,
						   Sampler_filter /*filter*/, Worker& /*worker*/,
						   float3& li, float3& transmittance, float3& weight) {
	li = float3(0.f);
	transmittance = float3(1.f);
	weight = float3(1.f);
	return false;
}

Factory::Factory(const take::Settings& settings, uint32_t num_integrators) :
	take_settings_(settings), num_integrators_(num_integrators) {}

Factory::~Factory() {}

}
