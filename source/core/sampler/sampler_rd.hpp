#ifndef SU_CORE_SAMPLER_RD_HPP
#define SU_CORE_SAMPLER_RD_HPP

#include "sampler.hpp"

namespace sampler {

class alignas(64) RD : public Sampler {
  public:
    RD(rnd::Generator& rng) noexcept;

    ~RD() noexcept override final;

    Camera_sample generate_camera_sample(int2 pixel, uint32_t index) noexcept override final;

    float2 generate_sample_2D(uint32_t dimension = 0) noexcept override final;

    float generate_sample_1D(uint32_t dimension = 0) noexcept override final;

    size_t num_bytes() const noexcept override final;

  private:
    void on_resize() noexcept override final;

    void on_start_pixel() noexcept override final;

    float2* seeds_2D_;
    float*  seeds_1D_;
};

class RD_factory : public Factory {
  public:
    RD_factory(uint32_t num_samplers) noexcept;

    ~RD_factory() noexcept override final;

    Sampler* create(uint32_t id, rnd::Generator& rng) const noexcept override final;

  private:
    RD* samplers_;
};

}  // namespace sampler

#endif
