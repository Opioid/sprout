#ifndef SU_BASE_MATH_DISTRIBUTION_DISTRIBUTION_3D_HPP
#define SU_BASE_MATH_DISTRIBUTION_DISTRIBUTION_3D_HPP

#include "distribution_2d.hpp"
#include "math/vector3.hpp"

namespace math {

class Distribution_3D {
  public:
    using Distribution_impl = Distribution_2D::Distribution_impl;

    Distribution_3D() noexcept;

    ~Distribution_3D() noexcept;

    Distribution_2D* allocate(uint32_t num) noexcept;

    void init() noexcept;

    float integral() const noexcept;

    struct Continuous {
        float3 uvw;
        float  pdf;
    };
    Continuous sample_continuous(float3 const& r3) const noexcept;

    float pdf(float3 const& uvw) const noexcept;

    size_t num_bytes() const noexcept;

  private:
    Distribution_impl marginal_;

    Distribution_2D* conditional_;
    uint32_t         conditional_size_;

    float    conditional_sizef_;
    uint32_t conditional_max_;
};

}  // namespace math

#endif
