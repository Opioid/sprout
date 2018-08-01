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

    Debug(rnd::Generator& rng, take::Settings const& take_settings, Settings const& settings) noexcept;

    void prepare(Scene const& scene, uint32_t num_samples_per_pixel) noexcept override final;

    void resume_pixel(uint32_t sample, rnd::Generator& scramble) noexcept override final;

    float3 li(Ray& ray, Intersection& intersection, Worker& worker) noexcept override final;

    size_t num_bytes() const noexcept override final;

  private:
    Settings settings_;

    sampler::Random sampler_;
};

class Debug_factory final : public Factory {
  public:
    Debug_factory(take::Settings const& take_settings, uint32_t num_integrators,
                  Debug::Settings::Vector vector) noexcept;

    ~Debug_factory() noexcept override final;

    Integrator* create(uint32_t id, rnd::Generator& rng) const noexcept override final;

  private:
    Debug* integrators_;

    Debug::Settings settings_;
};

}  // namespace rendering::integrator::surface
