#ifndef SU_CORE_SAMPLER_GOLDEN_RATIO_HPP
#define SU_CORE_SAMPLER_GOLDEN_RATIO_HPP

#include "sampler.hpp"

namespace sampler {

class alignas(64) Golden_ratio final : public Sampler {
  public:
    Golden_ratio(rnd::Generator& rng) noexcept;

    ~Golden_ratio() noexcept override final;

    float2 generate_sample_2D(uint32_t dimension = 0) noexcept override final;

    float generate_sample_1D(uint32_t dimension = 0) noexcept override final;

  private:
    void on_resize() noexcept override final;

    void on_start_pixel() noexcept override final;

    void generate_2D(uint32_t dimension) noexcept;
    void generate_1D(uint32_t dimension) noexcept;

    float2* samples_2D_;
    float*  samples_1D_;
};

extern template class Typed_pool<Golden_ratio>;

using Golden_ratio_pool = Typed_pool<Golden_ratio>;

}  // namespace sampler

#endif
