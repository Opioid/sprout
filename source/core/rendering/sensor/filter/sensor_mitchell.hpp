#ifndef SU_CORE_RENDERING_SENSOR_FILTER_MITCHELL_HPP
#define SU_CORE_RENDERING_SENSOR_FILTER_MITCHELL_HPP

#include "base/math/interpolated_function_1d.hpp"
#include "base/math/vector3.hpp"

namespace rendering::sensor::filter {

class Mitchell {
  public:
    Mitchell(float radius, float b, float c) noexcept;

    Mitchell(Mitchell&& other) noexcept;

    float radius() const noexcept;

    float evaluate(float d) const noexcept;

    float evaluate(float2 p) const noexcept;

  private:
    float mitchell(float x) const noexcept;

    float radius_;
    float radius_inv_;

    Interpolated_function_1D<float> mitchell_;

    float b_;
    float c_;
};

}  // namespace rendering::sensor::filter

#endif
