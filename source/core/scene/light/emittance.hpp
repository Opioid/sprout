#pragma once

#include "base/math/vector.hpp"

namespace light {

class Emittance {

public:

	// Radiometric quantities
	enum class Quantity {
		Undefined,
		Flux,
		Intensity,
		Radiosty,
		Radiance
	};

	void set_flux(math::pfloat3 color, float value);

	void set_intensity(math::pfloat3 intensity);
	void set_intensity(math::pfloat3 color, float value);

	void set_exitance(math::pfloat3 color, float value);

	void set_radiance(math::pfloat3 radiance);
	void set_luminance(math::pfloat3 color, float value);

	math::float3 radiance(float area) const;

private:

	math::float3 value_;

	Quantity quantity_;
};

}
