#ifndef SU_BASE_SPECTRUM_INTERPOLATED_HPP
#define SU_BASE_SPECTRUM_INTERPOLATED_HPP

#include <cstddef>
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

}  // namespace spectrum

#endif
