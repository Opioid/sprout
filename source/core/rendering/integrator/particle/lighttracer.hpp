#ifndef SU_RENDERING_INTEGRATOR_PARTICLE_LIGHTTRACER_HPP
#define SU_RENDERING_INTEGRATOR_PARTICLE_LIGHTTRACER_HPP

#include "rendering/integrator/integrator.hpp"
#include "sampler/sampler_golden_ratio.hpp"
#include "sampler/sampler_random.hpp"

namespace sampler {
struct Camera_sample_to;
}

namespace scene::prop {
class Interface_stack;
}

namespace scene::shape {
struct Sample_from;
}

namespace scene::light {
class Light;
}

namespace rendering {

class Worker;

namespace integrator::particle {

class alignas(64) Lighttracer final : public Integrator {
  public:
    using Interface_stack  = scene::prop::Interface_stack;
    using Camera_sample_to = sampler::Camera_sample_to;
    using Sample_from      = scene::shape::Sample_from;

    struct Settings {
        uint32_t min_bounces;
        uint32_t max_bounces;
    };

    Lighttracer(rnd::Generator& rng, take::Settings const& take_settings,
                Settings const& settings) noexcept;

    ~Lighttracer() noexcept override final;

    void prepare(Scene const& scene, uint32_t num_samples_per_pixel) noexcept override final;

    void start_pixel() noexcept override final;

    void li(uint32_t frame, int4 const& bounds, Worker& worker,
            Interface_stack const& initial_stack) noexcept;

    size_t num_bytes() const noexcept override final;

  private:
    bool generate_light_ray(uint32_t frame, Worker& worker, Ray& ray, Light& light_out,
                            Sample_from& light_sample) noexcept;

    sampler::Sampler& material_sampler(uint32_t bounce) noexcept;

    Settings const settings_;

    sampler::Random sampler_;

    static uint32_t constexpr Num_material_samplers = 3;
    sampler::Golden_ratio material_samplers_[Num_material_samplers];
};

class Lighttracer_factory final {
  public:
    Lighttracer_factory(take::Settings const& take_settings, uint32_t num_integrators,
                        uint32_t min_bounces, uint32_t max_bounces) noexcept;

    ~Lighttracer_factory() noexcept;

    Lighttracer* create(uint32_t id, rnd::Generator& rng) const noexcept;

    uint32_t max_sample_depth() const noexcept;

  private:
    take::Settings const& take_settings_;

    Lighttracer* integrators_;

    Lighttracer::Settings settings_;
};

}  // namespace integrator::particle
}  // namespace rendering

#endif
