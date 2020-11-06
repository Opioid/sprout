#ifndef SU_CORE_SAMPLER_RANDOM_HPP
#define SU_CORE_SAMPLER_RANDOM_HPP

#include "sampler.hpp"

namespace sampler {

class alignas(32) Random : public Sampler {
  public:
    Random();

    float2 sample_2D(RNG& rng, uint32_t dimension = 0) final;

    float sample_1D(RNG& rng, uint32_t dimension = 0) final;

  private:
    void on_resize() final;

    void on_start_pixel(RNG& rng) final;
};

extern template class Typed_pool<Random>;

using Random_pool = Typed_pool<Random>;

}  // namespace sampler

#endif
