#ifndef SU_RENDERING_INTEGRATOR_PARTICLE_IMPORTANCE_HPP
#define SU_RENDERING_INTEGRATOR_PARTICLE_IMPORTANCE_HPP

#include "base/math/distribution/distribution_2d.hpp"
#include "base/math/vector3.hpp"

#include <vector>

namespace scene {

namespace prop {
struct Intersection;
}  // namespace prop

class Scene;

class Worker;

}  // namespace scene

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

    void prepare_sampling(uint32_t id, float* buffer, scene::Scene const& scene, Threads& threads);

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
    using Intersection = scene::prop::Intersection;
    using Worker       = scene::Worker;

    Importance_cache();

    ~Importance_cache();

    void init(scene::Scene const& scene);

    void clear();

    void set_eye_position(float3_p eye);

    void set_training(bool training);

    void prepare_sampling(scene::Scene const& scene, Threads& threads);

    void increment(uint32_t light_id, float2 uv);

    void increment(uint32_t light_id, float2 uv, Intersection const& isec, uint64_t time,
                   float weight, Worker const& worker);

    Importance const& importance(uint32_t light_id) const;

  private:
    float3 eye_;

    bool training_;

    std::vector<Importance> importances_;

    float* buffer_;
};

}  // namespace rendering::integrator::particle

#endif
