#ifndef SU_BASE_SPECTRUM_DISCRETE_INL
#define SU_BASE_SPECTRUM_DISCRETE_INL

#include "discrete.hpp"
#include "math/vector3.inl"
#include "xyz.hpp"

namespace spectrum {

template <int32_t N>
Discrete_spectral_power_distribution<N>::Discrete_spectral_power_distribution(float s) {
    for (int32_t i = 0; i < N; ++i) {
        values_[i] = s;
    }
}

template <int32_t N>
Discrete_spectral_power_distribution<N>::Discrete_spectral_power_distribution(
    Interpolated const& interpolated) {
    for (int32_t i = 0; i < N; ++i) {
        float const a = wavelengths_[i];
        float const b = wavelengths_[i + 1];

        values_[i] = interpolated.integrate(a, b) / (b - a);
    }
}

template <int32_t N>
Discrete_spectral_power_distribution<N>::Discrete_spectral_power_distribution(float3_p rgb)
    : Discrete_spectral_power_distribution(0.f) {
    float const r = rgb[0];
    float const g = rgb[1];
    float const b = rgb[2];

    if (r <= g && r <= b) {
        *this += r * rgb_reflector_to_spectrum_white_;
        if (g <= b) {
            *this += (g - r) * rgb_reflector_to_spectrum_cyan_;
            *this += (b - g) * rgb_reflector_to_spectrum_blue_;
        } else {
            *this += (b - r) * rgb_reflector_to_spectrum_cyan_;
            *this += (g - b) * rgb_reflector_to_spectrum_green_;
        }
    } else if (g <= r && g <= b) {
        *this += g * rgb_reflector_to_spectrum_white_;
        if (r <= b) {
            *this += (r - g) * rgb_reflector_to_spectrum_magenta_;
            *this += (b - r) * rgb_reflector_to_spectrum_blue_;
        } else {
            *this += (b - g) * rgb_reflector_to_spectrum_magenta_;
            *this += (r - b) * rgb_reflector_to_spectrum_red_;
        }
    } else {
        *this += b * rgb_reflector_to_spectrum_white_;
        if (r <= g) {
            *this += (r - b) * rgb_reflector_to_spectrum_yellow_;
            *this += (g - r) * rgb_reflector_to_spectrum_green_;
        } else {
            *this += (g - b) * rgb_reflector_to_spectrum_yellow_;
            *this += (r - g) * rgb_reflector_to_spectrum_red_;
        }
    }

    *this *= .94f;
}

template <int32_t N>
float constexpr Discrete_spectral_power_distribution<N>::value(int32_t bin) const {
    return values_[bin];
}

template <int32_t N>
void Discrete_spectral_power_distribution<N>::set_bin(int32_t bin, float value) {
    values_[bin] = value;
}

template <int32_t N>
float3 constexpr Discrete_spectral_power_distribution<N>::XYZ() const {
    float3 xyz(0.f);
    for (int32_t i = 0; i < N; ++i) {
        xyz += values_[i] * cie_[i];
    }

    return step_ * xyz;
}

template <int32_t N>
float3 constexpr Discrete_spectral_power_distribution<N>::normalized_XYZ() const {
    float constexpr Normalization = 1.f / 106.856895f;
    return Normalization * XYZ();
}

template <int32_t N>
void Discrete_spectral_power_distribution<N>::init(float start_wavelength, float end_wavelength) {
    float const step = (end_wavelength - start_wavelength) / float(N);

    // initialize the wavelengths ranges of the bins
    for (int32_t i = 0; i < N + 1; ++i) {
        wavelengths_[i] = start_wavelength + float(i) * step;
    }

    step_ = step;

    Interpolated const CIE_X(CIE_XYZ_Num, CIE_Wavelengths_360_830_1nm, CIE_X_360_830_1nm);
    Interpolated const CIE_Y(CIE_XYZ_Num, CIE_Wavelengths_360_830_1nm, CIE_Y_360_830_1nm);
    Interpolated const CIE_Z(CIE_XYZ_Num, CIE_Wavelengths_360_830_1nm, CIE_Z_360_830_1nm);

    Discrete_spectral_power_distribution<N> const cie_x(CIE_X);
    Discrete_spectral_power_distribution<N> const cie_y(CIE_Y);
    Discrete_spectral_power_distribution<N> const cie_z(CIE_Z);

    for (int32_t i = 0; i < N; ++i) {
        cie_[i][0] = cie_x.values_[i];
        cie_[i][1] = cie_y.values_[i];
        cie_[i][2] = cie_z.values_[i];
    }

    rgb_reflector_to_spectrum_white_ = Interpolated(Num_wavelengths, RGB_to_spectrum_wavelengths,
                                                    RGB_reflector_to_spectrum_white);

    rgb_reflector_to_spectrum_cyan_ = Interpolated(Num_wavelengths, RGB_to_spectrum_wavelengths,
                                                   RGB_reflector_to_spectrum_cyan);

    rgb_reflector_to_spectrum_magenta_ = Interpolated(Num_wavelengths, RGB_to_spectrum_wavelengths,
                                                      RGB_reflector_to_spectrum_magenta);

    rgb_reflector_to_spectrum_yellow_ = Interpolated(Num_wavelengths, RGB_to_spectrum_wavelengths,
                                                     RGB_reflector_to_spectrum_yellow);

    rgb_reflector_to_spectrum_red_ = Interpolated(Num_wavelengths, RGB_to_spectrum_wavelengths,
                                                  RGB_reflector_to_spectrum_red);

    rgb_reflector_to_spectrum_green_ = Interpolated(Num_wavelengths, RGB_to_spectrum_wavelengths,
                                                    RGB_reflector_to_spectrum_green);

    rgb_reflector_to_spectrum_blue_ = Interpolated(Num_wavelengths, RGB_to_spectrum_wavelengths,
                                                   RGB_reflector_to_spectrum_blue);
}

template <int32_t N>
int32_t constexpr Discrete_spectral_power_distribution<N>::num_bands() {
    return N;
}

template <int32_t N>
float constexpr Discrete_spectral_power_distribution<N>::wavelength_center(int32_t bin) {
    return (wavelengths_[bin] + wavelengths_[bin + 1]) * 0.5f;
}

template <int32_t N>
float constexpr Discrete_spectral_power_distribution<N>::start_wavelength() {
    return wavelengths_[0];
}

template <int32_t N>
float constexpr Discrete_spectral_power_distribution<N>::end_wavelength() {
    return wavelengths_[N];
}

template <int32_t N>
float3 Discrete_spectral_power_distribution<N>::cie_[N];

template <int32_t N>
float Discrete_spectral_power_distribution<N>::wavelengths_[N + 1];

template <int32_t N>
float Discrete_spectral_power_distribution<N>::step_;

template <int32_t N>
Discrete_spectral_power_distribution<N>
    Discrete_spectral_power_distribution<N>::rgb_reflector_to_spectrum_white_;

template <int32_t N>
Discrete_spectral_power_distribution<N>
    Discrete_spectral_power_distribution<N>::rgb_reflector_to_spectrum_cyan_;

template <int32_t N>
Discrete_spectral_power_distribution<N>
    Discrete_spectral_power_distribution<N>::rgb_reflector_to_spectrum_magenta_;

template <int32_t N>
Discrete_spectral_power_distribution<N>
    Discrete_spectral_power_distribution<N>::rgb_reflector_to_spectrum_yellow_;

template <int32_t N>
Discrete_spectral_power_distribution<N>
    Discrete_spectral_power_distribution<N>::rgb_reflector_to_spectrum_red_;

template <int32_t N>
Discrete_spectral_power_distribution<N>
    Discrete_spectral_power_distribution<N>::rgb_reflector_to_spectrum_green_;

template <int32_t N>
Discrete_spectral_power_distribution<N>
    Discrete_spectral_power_distribution<N>::rgb_reflector_to_spectrum_blue_;

template <int32_t N>
Discrete_spectral_power_distribution<N>& operator+=(
    Discrete_spectral_power_distribution<N>& a, Discrete_spectral_power_distribution<N> const& b) {
    for (int32_t i = 0; i < N; ++i) {
        a.values_[i] += b.values_[i];
    }

    return a;
}

template <int32_t N>
Discrete_spectral_power_distribution<N>& operator*=(Discrete_spectral_power_distribution<N>& a,
                                                    float                                    s) {
    for (int32_t i = 0; i < N; ++i) {
        a.values_[i] *= s;
    }

    return a;
}

template <int32_t N>
Discrete_spectral_power_distribution<N> operator*(
    float s, Discrete_spectral_power_distribution<N> const& v) {
    Discrete_spectral_power_distribution<N> r;

    for (int32_t i = 0; i < N; ++i) {
        r.values_[i] = s * v.values_[i];
    }

    return r;
}

}  // namespace spectrum

#endif
