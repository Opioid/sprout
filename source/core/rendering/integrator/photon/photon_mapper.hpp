#ifndef SU_RENDERING_INTEGRATOR_PHOTON_MAPPER_HPP
#define SU_RENDERING_INTEGRATOR_PHOTON_MAPPER_HPP

#include "base/math/vector2.hpp"
#include "rendering/integrator/integrator.hpp"
#include "sampler/sampler_random.hpp"

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

    Mapper(rnd::Generator& rng, take::Settings const& take_settings, Settings const& settings);
    virtual ~Mapper() override;

    virtual void prepare(Scene const& scene, uint32_t num_photons) override final;

    virtual void resume_pixel(uint32_t sample, rnd::Generator& scramble) override final;

    uint32_t bake(Map& map, int32_t begin, int32_t end, float normalized_tick_offset,
                  float normalized_tick_slice, Worker& worker);

    virtual size_t num_bytes() const override final;

  private:
    uint32_t trace_photon(float normalized_tick_offset, float normalized_tick_slice, Worker& worker,
                          uint32_t max_photons, Photon* photons, uint32_t& num_photons);

    bool generate_light_ray(float normalized_tick_offset, float normalized_tick_slice,
                            Worker& worker, Ray& ray, scene::light::Light const** light,
                            scene::shape::Sample_from& light_sample);

    const Settings settings_;

    sampler::Random sampler_;

    Photon* photons_;
};

}  // namespace integrator::photon
}  // namespace rendering

#endif
