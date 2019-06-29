#ifndef SU_RENDERING_INTEGRATOR_PARTICLE_PHOTON_IMPORTANCE_HPP
#define SU_RENDERING_INTEGRATOR_PARTICLE_PHOTON_IMPORTANCE_HPP

#include <string_view>
#include "base/math/distribution/distribution_2d.hpp"
#include "base/math/vector2.hpp"

namespace thread {
class Pool;
}

namespace rendering::integrator::particle::photon {

class Importance {
  public:
    Importance() noexcept;

    ~Importance() noexcept;

    void increment(float2 uv) noexcept;

    Distribution_2D const& distribution() const noexcept;

    float denormalization_factor() const noexcept;

    void export_heatmap(std::string_view name) const noexcept;

    void prepare_sampling(thread::Pool& pool) noexcept;

  private:
    int2      dimensions_;
    uint32_t* importance_;

    int2   dimensions_back_;
    float2 dimensions_float_;

    Distribution_2D distribution_;
};

}  // namespace rendering::integrator::particle::photon

#endif
