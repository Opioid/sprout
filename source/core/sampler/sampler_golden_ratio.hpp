#ifndef SU_CORE_SAMPLER_GOLDEN_RATIO_HPP
#define SU_CORE_SAMPLER_GOLDEN_RATIO_HPP


#include "sampler.hpp"

namespace sampler {

class alignas(64) Golden_ratio final : public Sampler {
  public:
    Golden_ratio(rnd::Generator& rng) noexcept;

    ~Golden_ratio() noexcept override final;

    Camera_sample generate_camera_sample(int2 pixel, uint32_t index) noexcept override final;

    float2 generate_sample_2D(uint32_t dimension = 0) noexcept override final;

    float generate_sample_1D(uint32_t dimension = 0) noexcept override final;

    size_t num_bytes() const noexcept override final;

  private:
    void on_resize() noexcept override final;

    void on_resume_pixel(rnd::Generator& scramble) noexcept override final;

    float2* samples_2D_;
    float*  samples_1D_;
};

class Golden_ratio_factory final : public Factory {
  public:
    Golden_ratio_factory(uint32_t num_samplers) noexcept;

    ~Golden_ratio_factory() noexcept override final;

    Sampler* create(uint32_t id, rnd::Generator& rng) const noexcept override final;

  private:
    Golden_ratio* samplers_;
};

}  // namespace sampler

#endif
