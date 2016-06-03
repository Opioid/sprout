#pragma once

#include "spectrum.hpp"

namespace spectrum {

template<uint32_t N>
void Discrete_spectral_power_distribution<N>::set_bin(uint32_t bin, float value) {
	values_[bin] = value;
}

template<uint32_t N>
math::float3 Discrete_spectral_power_distribution<N>::xyz() const {
	math::float3 xyz(0.f);
	for (uint32_t i = 0; i < N; ++i) {
		xyz.x += cie_x_.values_[i] * values_[i];
		xyz.y += cie_y_.values_[i] * values_[i];
		xyz.z += cie_z_.values_[i] * values_[i];
	}

	xyz *= (end_wavelength() - start_wavelength()) / static_cast<float>(N);

	return xyz;
}

template<uint32_t N>
void Discrete_spectral_power_distribution<N>::init(float start_wavelength, float end_wavelength) {
	float step = (end_wavelength - start_wavelength) / static_cast<float>(N);

	// initialize the wavelengths ranges of the bins
	for (uint32_t i = 0; i < N + 1; ++i) {
		wavelengths_[i] = start_wavelength + static_cast<float>(i) * step;
	}
}

template<uint32_t N>
float Discrete_spectral_power_distribution<N>::start_wavelength() {
	return wavelengths_[0];
}

template<uint32_t N>
float Discrete_spectral_power_distribution<N>::end_wavelength() {
	return wavelengths_[N];
}

template<uint32_t N>
float Discrete_spectral_power_distribution<N>::wavelengths_[N  + 1];

}
