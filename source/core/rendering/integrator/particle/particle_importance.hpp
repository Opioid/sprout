#ifndef SU_RENDERING_INTEGRATOR_PARTICLE_IMPORTANCE_HPP
#define SU_RENDERING_INTEGRATOR_PARTICLE_IMPORTANCE_HPP

#include "base/math/distribution/distribution_2d.hpp"
#include "core/scene/scene.hpp"
#include "base/math/vector3.hpp"
#include "base/memory/array.hpp"

#include <vector>

namespace thread {
class Pool;
}

using Threads = thread::Pool;

namespace rendering::integrator::particle {

class Importance {
  public:
    Importance();

    ~Importance();

    void clear();

    bool valid() const;

    void increment(float2 uv, float weight);

    Distribution_2D const& distribution() const;

    float denormalization_factor() const;

    float total_weight() const;

    float prepare_sampling(uint32_t id, float* buffer, scene::Scene const& scene, Threads& threads);

    static int32_t constexpr Dimensions = 256;

  private:
    void filter(float* buffer, Threads& threads) const;

    struct Weight {
        float w;

        uint32_t c;
    };

    Weight* importance_;

    Distribution_2D distribution_;

    bool valid_;
};

class Importance_cache {
  public:
    Importance_cache();

    ~Importance_cache();

    void init(scene::Scene const& scene);

    void clear();

    void set_eye_position(float3_p eye);

    void set_training(bool training);

    void prepare_sampling(scene::Scene const& scene, Threads& threads);

    void increment(uint32_t light_id, float2 uv);

    void increment(uint32_t light_id, float2 uv, float3_p p, float weight);

    scene::Scene::Light_pick random_light(float random, scene::Scene const& scene) const;

    Importance const& importance(uint32_t light_id) const;

  private:
    float3 eye_;

    bool training_;

    std::vector<Importance> importances_;

    float* buffer_;

    memory::Array<float> relative_importances_;

    Distribution_1D light_distribution_;
};

}  // namespace rendering::integrator::particle

#endif
