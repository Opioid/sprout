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

	// lumen
	void set_flux(math::pfloat3 color, float value);

	// watt per unit solid angle (W / sr)
	void set_intensity(math::pfloat3 intensity);

	// lumen per unit solid angle (lm / sr == candela (cd))
	void set_intensity(math::pfloat3 color, float value);

	// lumen per unit projected area (lumen / m^2)
	void set_exitance(math::pfloat3 color, float value);

	// watt per unit solid angle per unit projected area (W / sr / m^2)
	void set_radiance(math::pfloat3 radiance);

	// lumen per unit solid angle per unit projected area (lm / sr / m^2 == cd / m^2)
	void set_luminance(math::pfloat3 color, float value);

	math::float3 radiance(float area) const;

private:

	math::float3 value_;

	Quantity quantity_;
};

}
