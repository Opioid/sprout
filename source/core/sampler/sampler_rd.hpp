#ifndef SU_CORE_SAMPLER_RD_HPP
#define SU_CORE_SAMPLER_RD_HPP

#include "sampler.hpp"

namespace sampler {

class alignas(64) RD final : public Buffered {
  public:
    RD(uint32_t num_dimensions_2D, uint32_t num_dimensions_1D, uint32_t max_samples);

    ~RD() final;

    float2 sample_2D(RNG& rng, uint32_t dimension = 0) final;

    float sample_1D(RNG& rng, uint32_t dimension = 0) final;

  private:
    void on_start_pixel(RNG& rng) final;

    void generate_2D(RNG& rng, uint32_t dimension);
    void generate_1D(RNG& rng, uint32_t dimension);

    static uint32_t constexpr Num_batch = 64;

    float* seeds_;
    float* samples_;

    uint32_t* consumed_;
};

extern template class Typed_pool<RD>;

using RD_pool = Typed_pool<RD>;

}  // namespace sampler

#endif
