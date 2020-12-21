#ifndef SU_CORE_SAMPLER_GOLDEN_RATIO_HPP
#define SU_CORE_SAMPLER_GOLDEN_RATIO_HPP

#include "sampler.hpp"

namespace sampler {

class alignas(32) Golden_ratio final : public Buffered {
  public:
    Golden_ratio(uint32_t num_dimensions_2D, uint32_t num_dimensions_1D);

    ~Golden_ratio() final;

    float2 sample_2D(RNG& rng, uint32_t dimension = 0) final;

    float sample_1D(RNG& rng, uint32_t dimension = 0) final;

  private:
    void on_resize() final;

    void on_start_pixel(RNG& rng) final;

    void generate_2D(RNG& rng, uint32_t dimension);
    void generate_1D(RNG& rng, uint32_t dimension);

    float* samples_;
};

extern template class Typed_pool<Golden_ratio>;

using Golden_ratio_pool = Typed_pool<Golden_ratio>;

}  // namespace sampler

#endif
