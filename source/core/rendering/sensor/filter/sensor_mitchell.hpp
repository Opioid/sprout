#ifndef SU_CORE_RENDERING_SENSOR_FILTER_MITCHELL_HPP
#define SU_CORE_RENDERING_SENSOR_FILTER_MITCHELL_HPP

#include "base/math/interpolated_function_1d.hpp"
#include "base/math/vector3.hpp"

namespace rendering::sensor::filter {

class Mitchell {
  public:
    Mitchell(float radius, float b, float c);

    Mitchell(Mitchell&& other) noexcept;

    float radius() const;

    float evaluate(float d) const;

    float evaluate(float2 p) const;

  private:
    float mitchell(float x) const;

    float radius_;
    float radius_inv_;

    Interpolated_function_1D<float> mitchell_;

    float b_;
    float c_;
};

}  // namespace rendering::sensor::filter

#endif
