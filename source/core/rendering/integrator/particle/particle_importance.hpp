#ifndef SU_RENDERING_INTEGRATOR_PARTICLE_IMPORTANCE_HPP
#define SU_RENDERING_INTEGRATOR_PARTICLE_IMPORTANCE_HPP

#include "base/math/distribution/distribution_2d.hpp"
#include "base/math/vector3.hpp"

#include <vector>

namespace scene {
class Scene;
}

namespace thread {
class Pool;
}

namespace rendering::integrator::particle {

class Importance {
  public:
    Importance() noexcept;

    ~Importance() noexcept;

    void increment(float2 uv, float weight) noexcept;

    Distribution_2D const& distribution() const noexcept;

    float denormalization_factor() const noexcept;

    void prepare_sampling(uint32_t id, float* buffer, thread::Pool& threads) noexcept;

    static int32_t constexpr Dimensions = 256;

  private:
    void filter(float* buffer, thread::Pool& threads) const noexcept;

    float* importance_;

    Distribution_2D distribution_;
};

class Importance_cache {
  public:
    Importance_cache() noexcept;

    ~Importance_cache() noexcept;

    void init(scene::Scene const& scene) noexcept;

    void set_eye_position(float3 const& eye) noexcept;

    void set_training(bool training) noexcept;

    void prepare_sampling(thread::Pool& threads) noexcept;

    void increment(uint32_t light_id, float2 uv) noexcept;

    void increment(uint32_t light_id, float2 uv, float3 const& p) noexcept;

    Importance const& importance(uint32_t light_id) const noexcept;

  private:
    float3 eye_;

    bool training_;

    std::vector<Importance> importances_;

    float* buffer_;
};

}  // namespace rendering::integrator::particle

#endif
