#ifndef SU_BASE_MATH_FILTER_GAUSSIAN_HPP
#define SU_BASE_MATH_FILTER_GAUSSIAN_HPP

#include <algorithm>
#include <cmath>

namespace math::filter {

class Gaussian_functor {
  public:
    Gaussian_functor(float squared_radius, float alpha) noexcept
        : exp_(std::exp(-alpha * squared_radius)), alpha_(alpha) {}

    float operator()(float squared_d) const noexcept {
        return std::max(0.f, std::exp(-alpha_ * squared_d) - exp_);
    }

  private:
    float exp_;
    float alpha_;
};

}  // namespace math::filter

#endif
