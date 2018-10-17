#ifndef SU_RENDERING_INTEGRATOR_PHOTON_MAPPER_HPP
#define SU_RENDERING_INTEGRATOR_PHOTON_MAPPER_HPP

#include "base/math/vector2.hpp"
#include "rendering/integrator/integrator.hpp"
#include "sampler/sampler_random.hpp"

namespace math {
struct AABB;
}

namespace scene::shape {
struct Sample_from;
}

namespace scene::light {
class Light;
}

namespace rendering {

class Worker;

namespace integrator::photon {

struct Photon;
class Map;

class Mapper : public Integrator {
  public:
    struct Settings {
        uint32_t max_bounces;

        bool indirect_caustics;
        bool full_light_path;
    };

    Mapper(rnd::Generator& rng, take::Settings const& take_settings,
           Settings const& settings) noexcept;

    ~Mapper() noexcept override;

    void prepare(Scene const& scene, uint32_t num_photons) noexcept override final;

    void start_pixel() noexcept override final;

    uint32_t bake(Map& map, int32_t begin, int32_t end, float normalized_tick_offset,
                  float normalized_tick_slice, Worker& worker) noexcept;

    size_t num_bytes() const noexcept override final;

  private:
    uint32_t trace_photon(float normalized_tick_offset, float normalized_tick_slice,
                          math::AABB const& bounds, bool infinite_world, Worker& worker,
                          uint32_t max_photons, Photon* photons, uint32_t& num_photons) noexcept;

    bool generate_light_ray(float normalized_tick_offset, float normalized_tick_slice,
                            math::AABB const& bounds, Worker& worker, Ray& ray,
                            scene::light::Light const** light,
                            scene::shape::Sample_from&  light_sample) noexcept;

    const Settings settings_;

    sampler::Random sampler_;

    Photon* photons_;
};

}  // namespace integrator::photon
}  // namespace rendering

#endif
