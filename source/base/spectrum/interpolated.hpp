#ifndef SU_BASE_SPECTRUM_INTERPOLATED_HPP
#define SU_BASE_SPECTRUM_INTERPOLATED_HPP

#include <cstddef>
#include <cstdint>

namespace spectrum {

class Interpolated {
  public:
    Interpolated(uint32_t len, float const* wavelengths, float const* intensities);

    ~Interpolated();

    float start_wavelength() const;

    float end_wavelength() const;

    float evaluate(float wl) const;

    float integrate(float a, float b) const;

  private:
    uint32_t num_elements_;

    float* wavelengths_;
    float* intensities_;
};

uint32_t constexpr Num_wavelengths = 64;

extern float const RGB_to_spectrum_wavelengths[Num_wavelengths];

// generated with a whitepoint of wx = .3333; wy = .3333;
extern float const RGB_reflector_to_spectrum_white[Num_wavelengths];
extern float const RGB_reflector_to_spectrum_cyan[Num_wavelengths];
extern float const RGB_reflector_to_spectrum_magenta[Num_wavelengths];
extern float const RGB_reflector_to_spectrum_yellow[Num_wavelengths];
extern float const RGB_reflector_to_spectrum_red[Num_wavelengths];
extern float const RGB_reflector_to_spectrum_green[Num_wavelengths];
extern float const RGB_reflector_to_spectrum_blue[Num_wavelengths];

}  // namespace spectrum

#endif
