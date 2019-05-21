#ifndef SU_CORE_RENDERING_SENSOR_FILTER_GAUSSIAN_HPP
#define SU_CORE_RENDERING_SENSOR_FILTER_GAUSSIAN_HPP

#include "base/math/interpolated_function.hpp"
#include "base/math/vector3.hpp"
#include "sensor_filter.hpp"

namespace rendering::sensor::filter {

class Gaussian : public Filter {
  public:
    Gaussian(float radius, float alpha);

    float radius() const override final;

    float evaluate(float d) const override final;

    float evaluate(float2 p) const override final;

  private:
    //  float gaussian(float d) const;

    float radius_;

    Interpolated_function<float> gaussian_;

    //    float exp_;

    //    float alpha_;
};
/*
class Gaussian_1 : public Filter {
  public:
    Gaussian_1(float radius, float alpha);

    float radius() const override final;

    float evaluate(float d) const override final;

    float evaluate(float2 p) const override final;

  private:
    float radius_;

    Interpolated_function<float> gaussian_;
};*/

}  // namespace rendering::sensor::filter

#endif
