#ifndef SU_CORE_SAMPLER_RANDOM_HPP
#define SU_CORE_SAMPLER_RANDOM_HPP

#include "sampler.hpp"

namespace sampler {

class Random : public Sampler {
  public:
    Random();

    Random(uint32_t num_dimensions_2D, uint32_t num_dimensions_1D, uint32_t max_samples);

    float2 sample_2D(RNG& rng, uint32_t dimension = 0) final;

    float sample_1D(RNG& rng, uint32_t dimension = 0) final;
};

extern template class Typed_pool<Random>;

using Random_pool = Typed_pool<Random>;

}  // namespace sampler

#endif
