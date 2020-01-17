#ifndef SU_RENDERING_INTEGRATOR_PARTICLE_IMPORTANCE_HPP
#define SU_RENDERING_INTEGRATOR_PARTICLE_IMPORTANCE_HPP

#include "base/math/distribution/distribution_2d.hpp"
#include "base/math/vector3.hpp"

#include <string_view>
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

    void export_heatmap(std::string_view name) const noexcept;

    void prepare_sampling(thread::Pool& threads) noexcept;

  private:
    int2   dimensions_;
    float* importance_;

    int2   dimensions_back_;
    float2 dimensions_float_;

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

    void increment_importance(uint32_t light_id, float2 uv) noexcept;
    void increment_importance(uint32_t light_id, float2 uv, float3 const& p) noexcept;

    Importance const& importance(uint32_t light_id) const noexcept;

    void export_importances() const noexcept;

  private:
    float3 eye_;

    bool training_;

    std::vector<Importance> importances_;
};

}  // namespace rendering::integrator::particle

#endif
