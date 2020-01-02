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

  private:
    void on_resize() noexcept override final;

    void on_start_pixel() noexcept override final;

    uint2 scramble_;
};

using LD_pool = Typed_pool<LD>;

}  // namespace sampler
