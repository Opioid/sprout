#ifndef SU_BASE_SPECTRUM_INTERPOLATED_HPP
#define SU_BASE_SPECTRUM_INTERPOLATED_HPP

#include <cstddef>
#include <cstdint>
#include <vector>

namespace spectrum {

class Interpolated {
  public:
    Interpolated() noexcept = default;

    Interpolated(float const* wavelengths, float const* intensities, size_t len) noexcept;

    float start_wavelength() const noexcept;
    float end_wavelength() const noexcept;

    float evaluate(float wl) const noexcept;

    float integrate(float a, float b) const noexcept;

  private:
    std::vector<float> wavelengths_;
    std::vector<float> intensities_;
};

uint32_t constexpr RGB_to_spectrum_num = 64;

extern float const RGB_to_spectrum_wavelengths[RGB_to_spectrum_num];

// generated with a whitepoint of wx = .3333; wy = .3333;
extern float const RGB_reflector_to_spectrum_white[RGB_to_spectrum_num];
extern float const RGB_reflector_to_spectrum_cyan[RGB_to_spectrum_num];
extern float const RGB_reflector_to_spectrum_magenta[RGB_to_spectrum_num];
extern float const RGB_reflector_to_spectrum_yellow[RGB_to_spectrum_num];
extern float const RGB_reflector_to_spectrum_red[RGB_to_spectrum_num];
extern float const RGB_reflector_to_spectrum_green[RGB_to_spectrum_num];
extern float const RGB_reflector_to_spectrum_blue[RGB_to_spectrum_num];

}  // namespace spectrum

#endif
