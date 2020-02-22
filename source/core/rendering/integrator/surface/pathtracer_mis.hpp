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

    Pathtracer_MIS(rnd::Generator& rng, Settings const& settings, bool progressive);

    ~Pathtracer_MIS() final;

    void prepare(Scene const& scene, uint32_t num_samples_per_pixel) final;

    void start_pixel() final;

    float4 li(Ray& ray, Intersection& intersection, Worker& worker,
              Interface_stack const& initial_stack) final;

  private:
    struct Result {
        float4 li;
        float3 photon_li;

        bool split_photon;
    };

    Result integrate(Ray& ray, Intersection& intersection, Worker& worker, bool integrate_photons);

    float3 sample_lights(Ray const& ray, Intersection& intersection,
                         Material_sample const& material_sample, bool evaluate_back, Filter filter,
                         Worker& worker);

    float3 evaluate_light(Light const& light, float light_weight, Ray const& history,
                          float3 const& p, uint32_t sampler_dimension, bool evaluate_back,
                          Intersection const& intersection, Material_sample const& material_sample,
                          Filter filter, Worker& worker);

    enum class State {
        Primary_ray       = 1 << 0,
        Treat_as_singular = 1 << 1,
        Is_translucent    = 1 << 2,
        Evaluate_back     = 1 << 3,
        Split_photon      = 1 << 4,
        Transparent       = 1 << 5
    };

    using Path_state = flags::Flags<State>;

    float3 connect_light(Ray const& ray, float3 const& geo_n, Intersection const& intersection,
                         Bxdf_sample sample_result, Path_state state, Filter filter, Worker& worker,
                         bool& pure_emissive);

    float3 connect_light_volume(float3 const& vli, Ray const& ray, Intersection const& intersection,
                                float bxdf_pdf, Path_state state, Worker& worker) const;

    sampler::Sampler& material_sampler(uint32_t bounce);

    sampler::Sampler& light_sampler(uint32_t bounce);

    Settings const settings_;

    sampler::Random sampler_;

    sampler::Pool* sampler_pool_;

    static uint32_t constexpr Num_dedicated_samplers = 3;

    sampler::Sampler* material_samplers_[Num_dedicated_samplers];

    sampler::Sampler* light_samplers_[Num_dedicated_samplers];
};

class Pathtracer_MIS_pool final : public Typed_pool<Pathtracer_MIS> {
  public:
    Pathtracer_MIS_pool(uint32_t num_integrators, bool progressive, uint32_t num_samples,
                        uint32_t min_bounces, uint32_t max_bounces, Light_sampling light_sampling,
                        bool enable_caustics, bool photons_only_through_specular);

    Integrator* get(uint32_t id, rnd::Generator& rng) const final;

  private:
    Pathtracer_MIS::Settings settings_;

    bool progressive_;
};

}  // namespace rendering::integrator::surface

#endif
