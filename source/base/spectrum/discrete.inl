#pragma once

#include "discrete.hpp"
#include "xyz.hpp"

namespace spectrum {

template<int32_t N>
Discrete_spectral_power_distribution<N>::Discrete_spectral_power_distribution(
		const Interpolated& interpolated) {
	for (int32_t i = 0; i < N; ++i) {
		float a = wavelengths_[i];
		float b = wavelengths_[i + 1];
		values_[i] = interpolated.integrate(a, b) / (b - a);
	}
}

template<int32_t N>
float Discrete_spectral_power_distribution<N>::value(int32_t bin) const {
	return values_[bin];
}

template<int32_t N>
void Discrete_spectral_power_distribution<N>::set_bin(int32_t bin, float value) {
	values_[bin] = value;
}

template<int32_t N>
void Discrete_spectral_power_distribution<N>::clear(float s) {
	for (uint32_t i = 0; i < N; ++i) {
		values_[i] = s;
	}
}

template<int32_t N>
float3 Discrete_spectral_power_distribution<N>::XYZ() const {
	float3 xyz(0.f);
	for (int32_t i = 0; i < N; ++i) {
		xyz += values_[i] * cie_[i];
	}

	xyz *= step_;

	return xyz;
}

template<int32_t N>
float3 Discrete_spectral_power_distribution<N>::normalized_XYZ() const {
	constexpr float normalization = 1.f / 106.856895f;
	return normalization * XYZ();
}

template<int32_t N>
void Discrete_spectral_power_distribution<N>::init(float start_wavelength, float end_wavelength) {
	float step = (end_wavelength - start_wavelength) / static_cast<float>(N);

	// initialize the wavelengths ranges of the bins
	for (int32_t i = 0; i < N + 1; ++i) {
		wavelengths_[i] = start_wavelength + static_cast<float>(i) * step;
	}

	step_ = step;

	const Interpolated CIE_X(CIE_Wavelengths_360_830_1nm, CIE_X_360_830_1nm, CIE_XYZ_Num);
	const Interpolated CIE_Y(CIE_Wavelengths_360_830_1nm, CIE_Y_360_830_1nm, CIE_XYZ_Num);
	const Interpolated CIE_Z(CIE_Wavelengths_360_830_1nm, CIE_Z_360_830_1nm, CIE_XYZ_Num);

	const Discrete_spectral_power_distribution<N> cie_x(CIE_X);
	const Discrete_spectral_power_distribution<N> cie_y(CIE_Y);
	const Discrete_spectral_power_distribution<N> cie_z(CIE_Z);

	for (int32_t i = 0; i < N; ++i) {
		cie_[i].x = cie_x.values_[i];
		cie_[i].y = cie_y.values_[i];
		cie_[i].z = cie_z.values_[i];
	}
}

template<int32_t N>
constexpr int32_t Discrete_spectral_power_distribution<N>::num_bands() {
	return N;
}

template<int32_t N>
float Discrete_spectral_power_distribution<N>::wavelength_center(int32_t bin) {
	return (wavelengths_[bin] + wavelengths_[bin + 1]) * 0.5f;
}


template<int32_t N>
float Discrete_spectral_power_distribution<N>::start_wavelength() {
	return wavelengths_[0];
}

template<int32_t N>
float Discrete_spectral_power_distribution<N>::end_wavelength() {
	return wavelengths_[N];
}

template<int32_t N>
float3 Discrete_spectral_power_distribution<N>::cie_[N];

template<int32_t N>
float Discrete_spectral_power_distribution<N>::wavelengths_[N  + 1];

template<int32_t N>
float Discrete_spectral_power_distribution<N>::step_;

}
