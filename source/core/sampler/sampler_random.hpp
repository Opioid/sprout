#ifndef SU_CORE_SAMPLER_RANDOM_HPP
#define SU_CORE_SAMPLER_RANDOM_HPP

#include "sampler.hpp"

namespace sampler {

class alignas(32) Random : public Sampler {
  public:
    Random(rnd::Generator& rng);

    float2 generate_sample_2D(uint32_t dimension = 0) final;

    float generate_sample_1D(uint32_t dimension = 0) final;

  private:
    void on_resize() final;

    void on_start_pixel() final;
};

extern template class Typed_pool<Random>;

using Random_pool = Typed_pool<Random>;

}  // namespace sampler

#endif
