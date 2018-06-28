#ifndef SU_RENDERING_INTEGRATOR_PHOTON_MAPPER_HPP
#define SU_RENDERING_INTEGRATOR_PHOTON_MAPPER_HPP

#include "base/math/vector2.hpp"
#include "rendering/integrator/integrator.hpp"
#include "sampler/sampler_random.hpp"

namespace rendering {

class Worker;

namespace integrator::photon {

struct Photon;
class Map;

class Mapper : public Integrator {
  public:
    struct Settings {
        uint32_t max_bounces;
        uint32_t max_photons_per_path;

        bool indirect_caustics;
        bool full_light_path;
    };

    Mapper(rnd::Generator& rng, take::Settings const& take_settings, Settings const& settings);
    virtual ~Mapper() override;

    virtual void prepare(Scene const& scene, uint32_t num_photons) override final;

    virtual void resume_pixel(uint32_t sample, rnd::Generator& scramble) override final;

    uint32_t bake(Map& map, uint2 range, Worker& worker);

    virtual size_t num_bytes() const override final;

  private:
    uint32_t trace_photon(Worker& worker, uint32_t max_photons, Photon* photons,
                          uint32_t& num_photons);

    bool generate_light_ray(Worker& worker, Ray& ray, float3& radiance);

    const Settings settings_;

    sampler::Random sampler_;

    Photon* photons_;
};

}  // namespace integrator::photon
}  // namespace rendering

#endif
