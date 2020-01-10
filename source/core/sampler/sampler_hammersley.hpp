#ifndef SU_CORE_SAMPLER_HAMMERSLEY_HPP
#define SU_CORE_SAMPLER_HAMMERSLEY_HPP

#include "sampler.hpp"

namespace sampler {

class alignas(64) Hammersley : public Sampler {
  public:
    Hammersley(rnd::Generator& rng) noexcept;

    Camera_sample generate_camera_sample(int2 pixel, uint32_t index) noexcept override final;

    float2 generate_sample_2D(uint32_t dimension = 0) noexcept override final;

    float generate_sample_1D(uint32_t dimension = 0) noexcept override final;

  private:
    void on_resize() noexcept override final;

    void on_start_pixel() noexcept override final;

    uint32_t scramble_;
};

extern template class Typed_pool<Hammersley>;

using Hammersley_pool = Typed_pool<Hammersley>;

}  // namespace sampler

#endif
