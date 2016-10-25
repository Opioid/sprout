#pragma once

#include "discrete.hpp"
#include "xyz.hpp"

namespace spectrum {

template<uint32_t N>
Discrete_spectral_power_distribution<N>::Discrete_spectral_power_distribution() {}

template<uint32_t N>
Discrete_spectral_power_distribution<N>::Discrete_spectral_power_distribution(
		const Interpolated& interpolated) {
/*	DiscreteSpectralPowerDistribution ret;
	_FORN
	{
		ret.v[i] = i_Spectrum.integrate(ms_pWavelengths[i], ms_pWavelengths[i + 1]) / (ms_pWavelengths[i + 1] - ms_pWavelengths[i]);
	}
	return ret;*/

	for (uint32_t i = 0; i < N; ++i) {
		float a = wavelengths_[i];
		float b = wavelengths_[i + 1];
		values_[i] = interpolated.integrate(a, b) / (b - a);
	}
}

template<uint32_t N>
void Discrete_spectral_power_distribution<N>::set_bin(uint32_t bin, float value) {
	values_[bin] = value;
}

template<uint32_t N>
void Discrete_spectral_power_distribution<N>::clear(float s) {
	for (uint32_t i = 0; i < N; ++i) {
		values_[i] = s;
	}
}

template<uint32_t N>
float3 Discrete_spectral_power_distribution<N>::XYZ() const {
	float3 xyz(0.f);
	for (uint32_t i = 0; i < N; ++i) {
		xyz.x += cie_x_.values_[i] * values_[i];
		xyz.y += cie_y_.values_[i] * values_[i];
		xyz.z += cie_z_.values_[i] * values_[i];


//		std::cout << "cie_x_.values_[" << i << "] " << cie_x_.values_[i] << std::endl;
//		std::cout << "values_[" << i << "] " << values_[i] << std::endl;
//		std::cout << "xyz " << xyz.x << ", " << xyz.y << ", " << xyz.z << std::endl;

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

	const Interpolated CIE_X(CIE_Wavelengths_360_830_1nm, CIE_X_360_830_1nm, CIE_XYZ_Num);
	const Interpolated CIE_Y(CIE_Wavelengths_360_830_1nm, CIE_Y_360_830_1nm, CIE_XYZ_Num);
	const Interpolated CIE_Z(CIE_Wavelengths_360_830_1nm, CIE_Z_360_830_1nm, CIE_XYZ_Num);

	cie_x_ = Discrete_spectral_power_distribution<N>(CIE_X);
	cie_y_ = Discrete_spectral_power_distribution<N>(CIE_Y);
	cie_z_ = Discrete_spectral_power_distribution<N>(CIE_Z);
}

template<uint32_t N>
constexpr uint32_t Discrete_spectral_power_distribution<N>::num_bands() {
	return N;
}

template<uint32_t N>
float Discrete_spectral_power_distribution<N>::wavelength_center(uint32_t bin) {
	return (wavelengths_[bin] + wavelengths_[bin + 1]) * 0.5f;
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

template<uint32_t N>
Discrete_spectral_power_distribution<N> Discrete_spectral_power_distribution<N>::cie_x_;

template<uint32_t N>
Discrete_spectral_power_distribution<N> Discrete_spectral_power_distribution<N>::cie_y_;

template<uint32_t N>
Discrete_spectral_power_distribution<N> Discrete_spectral_power_distribution<N>::cie_z_;

}
