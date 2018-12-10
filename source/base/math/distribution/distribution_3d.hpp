#ifndef SU_BASE_MATH_DISTRIBUTION_DISTRIBUTION_3D_HPP
#define SU_BASE_MATH_DISTRIBUTION_DISTRIBUTION_3D_HPP

#include "distribution_2d.hpp"
#include "math/vector3.hpp"

namespace math {

class Distribution_3D {
  public:
    using Distribution_impl = Distribution_2D::Distribution_impl;

    Distribution_3D();

    ~Distribution_3D();

    void init(std::vector<Distribution_2D>& conditional);

    float integral() const noexcept;

    struct Continuous {
        float3 uvw;
        float  pdf;
    };
    Continuous sample_continuous(float3 const& r3) const;

    float pdf(float3 const& uvw) const;

    size_t num_bytes() const;

  private:
    Distribution_impl marginal_;

    std::vector<Distribution_2D> conditional_;

    float    conditional_size_;
    uint32_t conditional_max_;
};

}  // namespace math

#endif
