#ifndef SU_BASE_SPECTRUM_INTERPOLATED_HPP
#define SU_BASE_SPECTRUM_INTERPOLATED_HPP

#include <cstddef>
#include <cstdint>

namespace spectrum {

class Interpolated {
  public:
    Interpolated(uint32_t len, float const* wavelengths, float const* intensities) noexcept;

    ~Interpolated() noexcept;

    float start_wavelength() const noexcept;
    float end_wavelength() const noexcept;

    float evaluate(float wl) const noexcept;

    float integrate(float a, float b) const noexcept;

  private:
    uint32_t num_elements_;

    float* wavelengths_;
    float* intensities_;
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
