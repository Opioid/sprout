#pragma once

#include "base/math/vector2.hpp"
#include "sampler.hpp"

namespace sampler {

class alignas(64) LD : public Sampler {
  public:
    LD(rnd::Generator& rng) noexcept;

    Camera_sample generate_camera_sample(int2 pixel, uint32_t index) noexcept override final;

    float2 generate_sample_2D(uint32_t dimension = 0) noexcept override final;

    float generate_sample_1D(uint32_t dimension = 0) noexcept override final;

    size_t num_bytes() const noexcept override final;

  private:
    void on_resize() noexcept override final;

    void on_start_pixel() noexcept override final;

    uint2 scramble_;
};

class LD_factory : public Factory {
  public:
    LD_factory(uint32_t num_samplers) noexcept;

    ~LD_factory() noexcept override final;

    Sampler* create(uint32_t id, rnd::Generator& rng) const noexcept override final;

  private:
    LD* samplers_;
};

}  // namespace sampler
