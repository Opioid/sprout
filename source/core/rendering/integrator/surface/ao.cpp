#include "ao.hpp"
#include "base/math/vector.inl"

namespace rendering {

math::float3 Ao::li(const Worker& worker, uint32_t subsample, math::Oray& ray, scene::Intersection& intersection) const {
	return math::float3(0.f, 0.f, 0.f);
}

Surface_integrator* Ao_factory::create(uint32_t id) const {
	return new Ao;
}

}
