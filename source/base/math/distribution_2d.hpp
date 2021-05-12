#ifndef SU_BASE_MATH_DISTRIBUTION_DISTRIBUTION_2D_HPP
#define SU_BASE_MATH_DISTRIBUTION_DISTRIBUTION_2D_HPP

#include "distribution_1d.hpp"
#include "math/vector2.hpp"

namespace math {

class Distribution_2D {
  public:
    Distribution_2D();

    Distribution_2D(Distribution_2D&& other);

    ~Distribution_2D();

    Distribution_1D* allocate(uint32_t num);

    Distribution_1D* conditional();

    bool empty() const;

    void init();

    float integral() const;

    struct Continuous {
        float2 uv;
        float  pdf;
    };
    Continuous sample_continuous(float2 r2) const;

    float pdf(float2 uv) const;

  private:
    Distribution_1D marginal_;

    uint32_t conditional_size_;

    float conditional_sizef_;

    Distribution_1D* conditional_;
};

}  // namespace math

#endif
