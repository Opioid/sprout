#pragma once

#include "interpolated.hpp"
#include "math/vector.hpp"

namespace spectrum {

template<uint32_t N>
class Discrete_spectral_power_distribution {

public:

	Discrete_spectral_power_distribution();

	Discrete_spectral_power_distribution(const Interpolated& interpolated);

	float value(uint32_t bin) const;

	void set_bin(uint32_t bin, float value);

	void clear(float s);

	float3 XYZ() const;

	float3 normalized_XYZ() const;

	static void init(float start_wavelength = 380.f, float end_wavelength = 720.f);

	static constexpr uint32_t num_bands();

	static float wavelength_center(uint32_t bin);

private:

	static float start_wavelength();
	static float end_wavelength();

	float values_[N];

	static float wavelengths_[N  + 1];

	static Discrete_spectral_power_distribution cie_x_;
	static Discrete_spectral_power_distribution cie_y_;
	static Discrete_spectral_power_distribution cie_z_;
};

}
