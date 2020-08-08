#pragma once

#include "sampler/sampler_random.hpp"
#include "surface_integrator.hpp"

namespace rendering::integrator::surface {

class alignas(64) Debug final : public Integrator {
  public:
    struct Settings {
        enum class Vector { Tangent, Bitangent, Geometric_normal, Shading_normal, UV };

        Vector vector;
    };

    Debug(Settings const& settings);

    void prepare(Scene const& scene, uint32_t num_samples_per_pixel) final;

    void start_pixel(rnd::Generator& rng) final;

    float4 li(Ray& ray, Intersection& intersection, Worker& worker,
              Interface_stack const& initial_stack) final;

  private:
    Settings settings_;

    sampler::Random sampler_;
};

class Debug_pool final : public Typed_pool<Debug> {
  public:
    Debug_pool(uint32_t num_integrators, Debug::Settings::Vector vector);

    Integrator* get(uint32_t id) const final;

  private:
    Debug::Settings settings_;
};

}  // namespace rendering::integrator::surface
