#ifndef SU_BASE_RANDOM_GENERATOR_HPP
#define SU_BASE_RANDOM_GENERATOR_HPP

#include <cstdint>

namespace rnd {

// http://www.pcg-random.org/

class Generator {
  public:
    Generator(uint64_t state = 0, uint64_t sequence = 0);

    uint32_t random_uint();
    float    random_float();

  private:
    uint32_t advance_pcg32();

    uint64_t state_;
    uint64_t inc_;
};

}  // namespace rnd

#endif
