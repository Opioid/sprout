#ifndef SU_CORE_SAMPLER_GOLDEN_RATIO_HPP
#define SU_CORE_SAMPLER_GOLDEN_RATIO_HPP

#include "sampler.hpp"

namespace sampler {

class alignas(32) Golden_ratio final : public Sampler {
  public:
    Golden_ratio();

    ~Golden_ratio() final;

    float2 generate_sample_2D(rnd::Generator& rng, uint32_t dimension = 0) final;

    float generate_sample_1D(rnd::Generator& rng, uint32_t dimension = 0) final;

  private:
    void on_resize() final;

    void on_start_pixel(rnd::Generator& rng) final;

    void generate_2D(rnd::Generator& rng, uint32_t dimension);
    void generate_1D(rnd::Generator& rng, uint32_t dimension);

    float* samples_;
};

extern template class Typed_pool<Golden_ratio>;

using Golden_ratio_pool = Typed_pool<Golden_ratio>;

}  // namespace sampler

#endif
