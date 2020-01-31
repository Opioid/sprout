#ifndef SU_CORE_RENDERING_SENSOR_FILTER_GAUSSIAN_HPP
#define SU_CORE_RENDERING_SENSOR_FILTER_GAUSSIAN_HPP

#include "base/math/interpolated_function_1d.hpp"
#include "base/math/vector3.hpp"

namespace rendering::sensor::filter {

class Gaussian {
  public:
    Gaussian(float radius, float alpha) noexcept;

    Gaussian(Gaussian&& other) noexcept;

    float radius() const noexcept;

    float evaluate(float d) const noexcept;

    float evaluate(float2 p) const noexcept;

  private:
    //  float gaussian(float d) const;

    float integral(uint32_t num_samples) const noexcept;

    float radius_;

    Interpolated_function_1D<float> gaussian_;

    //    float exp_;

    //    float alpha_;
};
/*
class Gaussian_1 : public Filter {
  public:
    Gaussian_1(float radius, float alpha);

    float radius() const final;

    float evaluate(float d) const final;

    float evaluate(float2 p) const final;

  private:
    float radius_;

    Interpolated_function_1D<float> gaussian_;
};*/

}  // namespace rendering::sensor::filter

#endif
