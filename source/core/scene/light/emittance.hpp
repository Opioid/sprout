#pragma once

#include "base/math/vector.hpp"

namespace light {

class Emittance {

public:

	enum class Metric {
		Radiance
	};

	void set_radiance(math::pfloat3 radiance);

	math::float3 radiance(float area) const;

private:

	math::float3 quantity_;

	Metric metric_;
};

}
