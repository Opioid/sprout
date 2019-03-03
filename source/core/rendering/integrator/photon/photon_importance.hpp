#ifndef SU_RENDERING_INTEGRATOR_PHOTON_IMPORTANCE_HPP
#define SU_RENDERING_INTEGRATOR_PHOTON_IMPORTANCE_HPP

#include <string_view>
#include "base/math/vector2.hpp"

namespace rendering::integrator::photon {

class Importance {
  public:
    Importance() noexcept;

    ~Importance() noexcept;

    void increment(float2 uv) noexcept;

    void export_heatmap(std::string_view name) const noexcept;

  private:
    int2   dimensions_;
    float* importance_;

    int2   dimensions_back_;
    float2 dimensions_float_;
};

}  // namespace rendering::integrator::photon

#endif
