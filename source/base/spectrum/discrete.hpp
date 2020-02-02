#ifndef SU_BASE_SPECTRUM_DISCRETE_HPP
#define SU_BASE_SPECTRUM_DISCRETE_HPP

#include "interpolated.hpp"
#include "math/vector3.hpp"

namespace spectrum {

template <int32_t N>
class Discrete_spectral_power_distribution {
  public:
    Discrete_spectral_power_distribution() = default;

    Discrete_spectral_power_distribution(float s);

    Discrete_spectral_power_distribution(Interpolated const& interpolated);

    Discrete_spectral_power_distribution(float3 const& rgb);

    float constexpr value(int32_t bin) const;

    void set_bin(int32_t bin, float value);

    void set_at_wavelength(float lambda, float value);

    float3 constexpr XYZ() const;

    float3 constexpr normalized_XYZ() const;

    static void init(float start_wavelength = 380.f, float end_wavelength = 720.f);

    static int32_t constexpr num_bands();

    static float constexpr wavelength_center(int32_t bin);

    static float constexpr start_wavelength();
    static float constexpr end_wavelength();

  private:
    alignas(16) float values_[N];

    static float3 cie_[N];

    static float wavelengths_[N + 1];

    static float step_;

    static Discrete_spectral_power_distribution rgb_reflector_to_spectrum_white_;
    static Discrete_spectral_power_distribution rgb_reflector_to_spectrum_cyan_;
    static Discrete_spectral_power_distribution rgb_reflector_to_spectrum_magenta_;
    static Discrete_spectral_power_distribution rgb_reflector_to_spectrum_yellow_;
    static Discrete_spectral_power_distribution rgb_reflector_to_spectrum_red_;
    static Discrete_spectral_power_distribution rgb_reflector_to_spectrum_green_;
    static Discrete_spectral_power_distribution rgb_reflector_to_spectrum_blue_;

    template <int32_t J>
    friend Discrete_spectral_power_distribution<J>& operator+=(
        Discrete_spectral_power_distribution<J>&       a,
        Discrete_spectral_power_distribution<J> const& b);

    template <int32_t J>
    friend Discrete_spectral_power_distribution<J>& operator*=(
        Discrete_spectral_power_distribution<J>& a, float s);

    template <int32_t J>
    friend Discrete_spectral_power_distribution<J> operator*(
        float s, Discrete_spectral_power_distribution<J> const& v);
};

}  // namespace spectrum

#endif
