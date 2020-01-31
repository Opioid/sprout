#ifndef SU_RENDERING_INTEGRATOR_PARTICLE_LIGHTTRACER_HPP
#define SU_RENDERING_INTEGRATOR_PARTICLE_LIGHTTRACER_HPP

#include "rendering/integrator/integrator.hpp"
#include "sampler/sampler_golden_ratio.hpp"
#include "sampler/sampler_random.hpp"

namespace math {
struct AABB;
}  // namespace math

namespace sampler {
struct Camera_sample_to;
}

namespace scene {

namespace camera {
class Camera;
}

namespace prop {
class Interface_stack;
}

namespace shape {
struct Sample_from;
}

}  // namespace scene

namespace rendering {

class Worker;

namespace integrator::particle {

class alignas(64) Lighttracer final : public Integrator {
  public:
    using Interface_stack  = scene::prop::Interface_stack;
    using Camera           = scene::camera::Camera;
    using Camera_sample_to = sampler::Camera_sample_to;
    using Sample_from      = scene::shape::Sample_from;

    struct Settings {
        uint32_t min_bounces;
        uint32_t max_bounces;

        bool indirect_caustics;
        bool full_light_path;
    };

    Lighttracer(rnd::Generator& rng, Settings const& settings) noexcept;

    ~Lighttracer() noexcept final;

    void prepare(Scene const& scene, uint32_t num_samples_per_pixel) noexcept final;

    void start_pixel() noexcept final;

    void li(uint32_t frame, int4 const& bounds, Worker& worker,
            Interface_stack const& initial_stack) noexcept;

  private:
    bool generate_light_ray(uint32_t frame, AABB const& bounds, Worker& worker, Ray& ray,
                            Light const*& light_out, uint32_t& light_id,
                            Sample_from& light_sample) noexcept;

    bool direct_camera(Camera const& camera, int4 const& bounds, float3 const& radiance,
                       Ray const& history, Intersection const& intersection,
                       Material_sample const& material_sample, Filter filter,
                       Worker& worker) noexcept;

    sampler::Sampler& material_sampler(uint32_t bounce) noexcept;

    Settings const settings_;

    sampler::Random sampler_;

    sampler::Random light_sampler_;

    static uint32_t constexpr Num_material_samplers = 3;
    sampler::Random material_samplers_[Num_material_samplers];
};

class Lighttracer_pool final {
  public:
    Lighttracer_pool(uint32_t num_integrators, uint32_t min_bounces, uint32_t max_bounces,
                     bool indirect_caustics, bool full_light_path) noexcept;

    ~Lighttracer_pool() noexcept;

    Lighttracer* get(uint32_t id, rnd::Generator& rng) const noexcept;

    uint32_t max_sample_depth() const noexcept;

  private:
    uint32_t num_integrators_;

    Lighttracer* integrators_;

    Lighttracer::Settings settings_;
};

}  // namespace integrator::particle
}  // namespace rendering

#endif
