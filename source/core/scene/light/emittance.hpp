#pragma once

#include "base/math/vector3.hpp"

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
	void set_flux(const float3& color, float value);

	// watt per unit solid angle (W / sr)
	void set_intensity(const float3& intensity);

	// lumen per unit solid angle (lm / sr == candela (cd))
	void set_intensity(const float3& color, float value);

	// lumen per unit projected area (lumen / m^2)
	void set_exitance(const float3& color, float value);

	// watt per unit solid angle per unit projected area (W / sr / m^2)
	void set_radiance(const float3& radiance);

	// lumen per unit solid angle per unit projected area (lm / sr / m^2 == cd / m^2)
	void set_luminance(const float3& color, float value);

	float3 radiance(float area) const;

private:

	float3 value_;

	Quantity quantity_;
};

}
