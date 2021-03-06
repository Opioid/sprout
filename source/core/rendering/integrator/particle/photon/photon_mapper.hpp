#ifndef SU_RENDERING_INTEGRATOR_PARTICLE_PHOTON_MAPPER_HPP
#define SU_RENDERING_INTEGRATOR_PARTICLE_PHOTON_MAPPER_HPP

#include "rendering/integrator/integrator.hpp"
#include "sampler/sampler_random.hpp"

namespace math {
struct AABB;
class Frustum;
}  // namespace math

namespace scene::shape {
struct Sample_from;
}

namespace scene::light {
class Light;
}

namespace rendering {

class Worker;

namespace integrator::particle::photon {

struct Photon;
class Map;

class Mapper : public Integrator {
  public:
    using AABB        = math::AABB;
    using Frustum     = math::Frustum;
    using Sample_from = scene::shape::Sample_from;

    struct Settings {
        uint32_t max_bounces;

        bool full_light_path;
    };

    Mapper(Settings const& settings);

    ~Mapper() override;

    void start_pixel(RNG& rng) final;

    uint32_t bake(Map& map, int32_t begin, int32_t end, uint32_t frame, uint32_t iteration,
                  Worker& worker);

    void export_importances() const;

  private:
    uint32_t trace_photon(uint32_t frame, AABB const& bounds, Frustum const& frustum,
                          bool infinite_world, bool caustics_only, Worker& worker,
                          uint32_t max_photons, Photon* photons, uint32_t& num_photons);

    bool generate_light_ray(uint32_t frame, AABB const& bounds, Worker& worker, Ray& ray,
                            uint32_t& light_id, Sample_from& light_sample);

    Settings const settings_;

    sampler::Random sampler_;

    Photon* photons_;
};

}  // namespace integrator::particle::photon
}  // namespace rendering

#endif
