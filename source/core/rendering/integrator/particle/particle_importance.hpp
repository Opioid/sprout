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

using Threads = thread::Pool;

namespace rendering::integrator::particle {

class Importance {
  public:
    Importance();

    ~Importance();

    void increment(float2 uv, float weight);

    Distribution_2D const& distribution() const;

    float denormalization_factor() const;

    void prepare_sampling(uint32_t id, float* buffer, Threads& threads);

    static int32_t constexpr Dimensions = 256;

  private:
    void filter(float* buffer, Threads& threads) const;

    struct Weight {
        float w;

        uint32_t c;
    };

    Weight* importance_;

    Distribution_2D distribution_;
};

class Importance_cache {
  public:
    Importance_cache();

    ~Importance_cache();

    void init(scene::Scene const& scene);

    void set_eye_position(float3 const& eye);

    void set_training(bool training);

    void prepare_sampling(Threads& threads);

    void increment(uint32_t light_id, float2 uv);

    void increment(uint32_t light_id, float2 uv, float3 const& p);

    Importance const& importance(uint32_t light_id) const;

  private:
    float3 eye_;

    bool training_;

    std::vector<Importance> importances_;

    float* buffer_;
};

}  // namespace rendering::integrator::particle

#endif
