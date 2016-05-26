#include "emittance.hpp"
#include "base/math/vector.inl"

namespace light {

void Emittance::set_radiance(math::pfloat3 radiance) {
	quantity_ = radiance;

	metric_ = Metric::Radiance;
}

math::float3 Emittance::radiance(float area) const {
	return quantity_;
}

}
