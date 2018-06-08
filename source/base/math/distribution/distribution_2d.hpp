#ifndef SU_BASE_MATH_DISTRIBUTION_DISTRIBUTION_2D_HPP
#define SU_BASE_MATH_DISTRIBUTION_DISTRIBUTION_2D_HPP

#include "distribution_1d.hpp"
#include "math/vector2.hpp"

namespace thread {
class Pool;
}

namespace math {

template <typename T>
class Distribution_t_2D {
  public:
    using Distribution_impl = T;

    void init(float const* data, int2 dimensions);
    void init(float const* data, int2 dimensions, thread::Pool& pool);
    void init(std::vector<Distribution_impl>& conditional);

    struct Continuous {
        float2 uv;
        float  pdf;
    };
    Continuous sample_continuous(float2 r2) const;

    float pdf(float2 uv) const;

    size_t num_bytes() const;

  private:
    Distribution_impl marginal_;

    std::vector<Distribution_impl> conditional_;

    float    conditional_size_;
    uint32_t conditional_max_;
};

using Distribution_2D = Distribution_t_2D<Distribution_implicit_pdf_lut_lin_1D>;

}  // namespace math

#endif
