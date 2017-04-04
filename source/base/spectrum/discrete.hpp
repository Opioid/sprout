#pragma once

#include "interpolated.hpp"
#include "math/vector3.hpp"

namespace spectrum {

template<int32_t N>
class Discrete_spectral_power_distribution {

public:

	Discrete_spectral_power_distribution() = default;

	Discrete_spectral_power_distribution(const Interpolated& interpolated);

	float value(int32_t bin) const;

	void set_bin(int32_t bin, float value);

	void clear(float s);

	float3 XYZ() const;

	float3 normalized_XYZ() const;

	static void init(float start_wavelength = 380.f, float end_wavelength = 720.f);

	static constexpr int32_t num_bands();

	static float wavelength_center(int32_t bin);

private:

	static float start_wavelength();
	static float end_wavelength();

	alignas(16) float values_[N];

	static alignas(16) float3 cie_[N];

	static float wavelengths_[N  + 1];

	static float step_;
};

}
