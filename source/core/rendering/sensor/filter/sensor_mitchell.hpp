#ifndef SU_CORE_RENDERING_SENSOR_FILTER_MITCHELL_HPP
#define SU_CORE_RENDERING_SENSOR_FILTER_MITCHELL_HPP

#include "base/math/interpolated_function.hpp"
#include "base/math/vector3.hpp"
#include "sensor_filter.hpp"

namespace rendering::sensor::filter {

class Mitchell : public Filter {
  public:
    Mitchell(float radius, float b, float c);

    float radius() const override final;

    float evaluate(float d) const override final;

    float evaluate(float2 p) const override final;

  private:
    float mitchell(float x) const;

    float radius_;
    float radius_inv_;

    Interpolated_function<float> mitchell_;
};

}  // namespace rendering::sensor::filter

#endif
