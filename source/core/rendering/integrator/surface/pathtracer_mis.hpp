#ifndef SU_RENDERING_INTEGRATOR_SURFACE_PATHTRACER_NG_HPP
#define SU_RENDERING_INTEGRATOR_SURFACE_PATHTRACER_NG_HPP

#include "base/flags/flags.hpp"
#include "base/math/vector3.hpp"
#include "base/math/vector4.hpp"
#include "sampler/sampler_random.hpp"
#include "surface_integrator.hpp"

namespace rendering::integrator::surface {

class alignas(64) Pathtracer_MIS final : public Integrator {
  public:
    struct Settings {
        uint32_t num_samples;
        uint32_t min_bounces;
        uint32_t max_bounces;

        Light_sampling light_sampling;

        bool avoid_caustics;
        bool photons_not_only_through_specular;
    };

    Pathtracer_MIS(Settings const& settings, uint32_t max_samples_per_pixel, bool progressive);

    ~Pathtracer_MIS() final;

    void start_pixel(RNG& rng) final;

    float4 li(Ray& ray, Intersection& isec, Worker& worker, Interface_stack const& initial_stack,
              AOV* aov) final;

  private:
    struct Result {
        float4 li;
        float3 photon_li;

        bool split_photon;
    };

    Result integrate(Ray& ray, Intersection& isec, Worker& worker, bool integrate_photons,
                     AOV* aov);

    float3 sample_lights(Ray const& ray, Intersection& isec, Material_sample const& mat_sample,
                         Filter filter, Worker& worker);

    float3 evaluate_light(Light const& light, float light_weight, Ray const& history, float3_p p,
                          uint32_t sampler_d, Intersection const& isec,
                          Material_sample const& mat_sample, Filter filter, Worker& worker);

    enum class State {
        Primary_ray       = 1 << 0,
        Treat_as_singular = 1 << 1,
        Is_translucent    = 1 << 2,
        Split_photon      = 1 << 3,
        Direct            = 1 << 4,
        From_subsurface   = 1 << 5
    };

    using Path_state = flags::Flags<State>;

    float3 connect_light(Ray const& ray, float3_p geo_n, Intersection const& isec,
                         Bxdf_sample sample_result, Path_state state, Filter filter, Worker& worker,
                         bool& pure_emissive);

    float connect_light_volume(Ray const& ray, float3_p geo_n, Intersection const& isec,
                               float bxdf_pdf, Path_state state, Worker& worker) const;

    sampler::Sampler& material_sampler(uint32_t bounce);

    sampler::Sampler& light_sampler(uint32_t bounce);

    bool splitting(uint32_t bounce) const;

    Settings const settings_;

    sampler::Random sampler_;

    sampler::Pool* sampler_pool_;

    static uint32_t constexpr Num_dedicated_samplers = 3;
};

class Pathtracer_MIS_pool final : public Typed_pool<Pathtracer_MIS, Integrator> {
  public:
    Pathtracer_MIS_pool(uint32_t num_integrators, bool progressive, uint32_t num_samples,
                        uint32_t min_bounces, uint32_t max_bounces, Light_sampling light_sampling,
                        bool enable_caustics, bool photons_only_through_specular);

    Integrator* create(uint32_t id, uint32_t max_samples_per_pixel) const final;

  private:
    Pathtracer_MIS::Settings settings_;

    bool progressive_;
};

}  // namespace rendering::integrator::surface

#endif
