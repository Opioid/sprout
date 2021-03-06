#ifndef SU_CORE_RENDERING_INTEGRATOR_SURFACE_DEBUG
#define SU_CORE_RENDERING_INTEGRATOR_SURFACE_DEBUG

#include "sampler/sampler_random.hpp"
#include "surface_integrator.hpp"

namespace rendering::integrator::surface {

class alignas(64) Debug final : public Integrator {
  public:
    struct Settings {
        enum class Value {
            Albedo,
            Roughness,
            Tangent,
            Bitangent,
            Geometric_normal,
            Shading_normal,
            UV,
            Splitting,
            Material_id,
            Light_id,
            Backface
        };

        Value value;
    };

    Debug(Settings const& settings, uint32_t max_samples_per_pixel);

    void start_pixel(RNG& rng) final;

    float4 li(Ray& ray, Intersection& isec, Worker& worker, Interface_stack const& initial_stack,
              AOV* aov) final;

  private:
    static float3 light_id(Ray& ray, Intersection& isec, Worker& worker);

    Settings settings_;

    sampler::Random sampler_;
};

class Debug_pool final : public Typed_pool<Debug> {
  public:
    Debug_pool(uint32_t num_integrators, Debug::Settings::Value value);

    Integrator* create(uint32_t id, uint32_t max_samples_per_pixel) const final;

  private:
    Debug::Settings settings_;
};

}  // namespace rendering::integrator::surface

#endif
