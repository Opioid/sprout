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
	void set_flux(float3_p color, float value);

	// watt per unit solid angle (W / sr)
	void set_intensity(float3_p intensity);

	// lumen per unit solid angle (lm / sr == candela (cd))
	void set_intensity(float3_p color, float value);

	// lumen per unit projected area (lumen / m^2)
	void set_exitance(float3_p color, float value);

	// watt per unit solid angle per unit projected area (W / sr / m^2)
	void set_radiance(float3_p radiance);

	// lumen per unit solid angle per unit projected area (lm / sr / m^2 == cd / m^2)
	void set_luminance(float3_p color, float value);

	float3 radiance(float area) const;

private:

	float3 value_;

	Quantity quantity_;
};

}
