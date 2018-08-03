#ifndef SU_BASE_SPECTRUM_DISCRETE_HPP
#define SU_BASE_SPECTRUM_DISCRETE_HPP

#include "interpolated.hpp"
#include "math/vector3.hpp"

namespace spectrum {

template <int32_t N>
class Discrete_spectral_power_distribution {
  public:
    Discrete_spectral_power_distribution() noexcept = default;

    Discrete_spectral_power_distribution(Interpolated const& interpolated) noexcept;

    float constexpr value(int32_t bin) const noexcept;

    void set_bin(int32_t bin, float value) noexcept;

    void set_at_wavelength(float lambda, float value) noexcept;

    void clear(float s) noexcept;

    float3 constexpr XYZ() const noexcept;

    float3 constexpr normalized_XYZ() const noexcept;

    static void init(float start_wavelength = 380.f, float end_wavelength = 720.f) noexcept;

    static int32_t constexpr num_bands() noexcept;

    static float constexpr wavelength_center(int32_t bin) noexcept;

    static float constexpr start_wavelength() noexcept;
    static float constexpr end_wavelength() noexcept;

  private:
    alignas(16) float values_[N];

    static float3 cie_[N];

    static float wavelengths_[N + 1];

    static float step_;
};

}  // namespace spectrum

#endif
