#ifndef SU_CORE_SAMPLER_RD_HPP
#define SU_CORE_SAMPLER_RD_HPP

#include "sampler.hpp"

namespace sampler {

class alignas(64) RD : public Sampler {
  public:
    RD(rnd::Generator& rng) noexcept;

    ~RD() noexcept final;

    float2 generate_sample_2D(uint32_t dimension = 0) noexcept final;

    float generate_sample_1D(uint32_t dimension = 0) noexcept final;

  private:
    void on_resize() noexcept final;

    void on_start_pixel() noexcept final;

    void generate_2D(uint32_t dimension) noexcept;
    void generate_1D(uint32_t dimension) noexcept;

    static uint32_t constexpr Num_batch = 16;

    float2* seeds_2D_;
    float*  seeds_1D_;

    float2* samples_2D_;
    float*  samples_1D_;

    uint32_t* consumed_2D_;
    uint32_t* consumed_1D_;
};

extern template class Typed_pool<RD>;

using RD_pool = Typed_pool<RD>;

}  // namespace sampler

#endif
