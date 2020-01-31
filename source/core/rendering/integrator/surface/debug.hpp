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

    Debug(rnd::Generator& rng, Settings const& settings) noexcept;

    void prepare(Scene const& scene, uint32_t num_samples_per_pixel) noexcept final;

    void start_pixel() noexcept final;

    float4 li(Ray& ray, Intersection& intersection, Worker& worker,
              Interface_stack const& initial_stack) noexcept final;

  private:
    Settings settings_;

    sampler::Random sampler_;
};

class Debug_pool final : public Typed_pool<Debug> {
  public:
    Debug_pool(uint32_t num_integrators, Debug::Settings::Vector vector) noexcept;

    Integrator* get(uint32_t id, rnd::Generator& rng) const noexcept final;

  private:
    Debug::Settings settings_;
};

}  // namespace rendering::integrator::surface
