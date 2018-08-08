#pragma once

#include "sampler.hpp"

namespace sampler {

class alignas(64) Hammersley : public Sampler {
  public:
    Hammersley(rnd::Generator& rng) noexcept;

    Camera_sample generate_camera_sample(int2 pixel, uint32_t index) noexcept override final;

    float2 generate_sample_2D(uint32_t dimension = 0) noexcept override final;

    float generate_sample_1D(uint32_t dimension = 0) noexcept override final;

    size_t num_bytes() const noexcept override final;

  private:
    void on_resize() noexcept override final;

    void on_resume_pixel(rnd::Generator& scramble) noexcept override final;

    uint32_t scramble_;
};

class Hammersley_factory : public Factory {
  public:
    Hammersley_factory(uint32_t num_samplers) noexcept;

    ~Hammersley_factory() noexcept override final;

    Sampler* create(uint32_t id, rnd::Generator& rng) const noexcept override final;

  private:
    Hammersley* samplers_;
};

}  // namespace sampler
