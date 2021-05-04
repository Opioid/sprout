#ifndef SU_CORE_RENDERING_SENSOR_FILTER_BLACKMAN_HPP
#define SU_CORE_RENDERING_SENSOR_FILTER_BLACKMAN_HPP

#include "base/math/interpolated_function_1d.hpp"
#include "base/math/vector.hpp"

namespace rendering::sensor::filter {

class Blackman {
  public:
    Blackman(float radius);

    Blackman(Blackman&& other) noexcept;

    ~Blackman();

    float evaluate(float d) const;

    float evaluate(float2 p) const;

  private:
    float integral(uint32_t num_samples, float radius) const;

    Interpolated_function_1D_N<float, 30> func_;
};

}  // namespace rendering::sensor::filter

#endif
