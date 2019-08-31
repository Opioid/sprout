#ifndef SU_BASE_MATH_DISTRIBUTION_DISTRIBUTION_2D_HPP
#define SU_BASE_MATH_DISTRIBUTION_DISTRIBUTION_2D_HPP

#include "distribution_1d.hpp"
#include "math/vector2.hpp"

namespace math {

template <typename T>
class Distribution_t_2D {
  public:
    using Distribution_impl = T;

    Distribution_t_2D() noexcept;

    ~Distribution_t_2D() noexcept;

    Distribution_impl* allocate(uint32_t num) noexcept;

    Distribution_impl* conditional() noexcept;

    bool empty() const noexcept;

    void init() noexcept;

    float integral() const noexcept;

    struct Continuous {
        float2 uv;
        float  pdf;
    };
    Continuous sample_continuous(float2 r2) const noexcept;

    float pdf(float2 uv) const noexcept;

    size_t num_bytes() const noexcept;

  private:
    Distribution_impl marginal_;

    uint32_t           conditional_size_;
    Distribution_impl* conditional_;

    float    conditional_sizef_;
    uint32_t conditional_max_;
};

extern template class Distribution_t_2D<Distribution_implicit_pdf_lut_lin_1D>;

using Distribution_2D = Distribution_t_2D<Distribution_implicit_pdf_lut_lin_1D>;

}  // namespace math

#endif
