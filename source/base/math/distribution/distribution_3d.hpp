#ifndef SU_BASE_MATH_DISTRIBUTION_DISTRIBUTION_3D_HPP
#define SU_BASE_MATH_DISTRIBUTION_DISTRIBUTION_3D_HPP

#include "distribution_2d.hpp"
#include "math/vector4.hpp"

namespace math {

class Distribution_3D {
  public:
    Distribution_3D();

    ~Distribution_3D();

    Distribution_2D* allocate(uint32_t num);

    void init();

    float integral() const;

    float4 sample_continuous(float3 const& r3) const;

    float pdf(float3 const& uvw) const;

  private:
    Distribution_1D marginal_;

    uint32_t conditional_size_;

    Distribution_2D* conditional_;

    float    conditional_sizef_;
    uint32_t conditional_max_;
};

}  // namespace math

#endif
