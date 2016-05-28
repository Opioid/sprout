#pragma once

#include "base/math/vector.hpp"

namespace light {

class Radiometry {

public:

	enum class Quantity {
		Intensity,
		Radiance
	};

	void set_intensity(math::pfloat3 intensity);
	void set_radiance(math::pfloat3 radiance);

	math::float3 radiance(float area) const;

private:

	math::float3 value_;

	Quantity quantity_;
};

}
