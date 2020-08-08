#ifndef SU_CORE_RENDERING_SENSOR_FILTER_GAUSSIAN_HPP
#define SU_CORE_RENDERING_SENSOR_FILTER_GAUSSIAN_HPP

#include "base/math/interpolated_function_1d.hpp"
#include "base/math/vector3.hpp"

namespace rendering::sensor::filter {

class Gaussian {
  public:
    Gaussian(float radius, float alpha);

    Gaussian(Gaussian&& other) noexcept;

    ~Gaussian();

    float evaluate(float d) const;

    float evaluate(float2 p) const;

  private:
    //  float gaussian(float d) const;

    float integral(uint32_t num_samples, float radius) const;

    Interpolated_function_1D_N<float, 30> gaussian_;
};

/*
class Gaussian_1 {
  public:
    Gaussian_1(float radius, float alpha);

    ~Gaussian_1();

    float radius() const;

    float evaluate(float d) const;

    float evaluate(float2 p) const;

  private:
    float integral(uint32_t num_samples) const;

    float radius_;

    Interpolated_function_1D<float> gaussian_;
};

class Reference_gaussian {
  public:
    Reference_gaussian(float radius, float alpha);

    float radius() const;

    float evaluate(float d) const;

    float evaluate(float2 p) const;

  private:
    float integral(uint32_t num_samples) const;

    float radius_;

    float exp_;

    float alpha_;

    float integral_;
};
*/
}  // namespace rendering::sensor::filter

#endif
