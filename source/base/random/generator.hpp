#ifndef SU_BASE_RANDOM_GENERATOR_HPP
#define SU_BASE_RANDOM_GENERATOR_HPP

#include <cstdint>

namespace rnd {

// http://www.pcg-random.org/

class Generator {
  public:
    Generator() noexcept;

    Generator(uint64_t state, uint64_t sequence) noexcept;

    void start(uint64_t state = 0, uint64_t sequence = 0) noexcept;

    uint32_t random_uint() noexcept;

    float random_float() noexcept;

  private:
    uint32_t advance_pcg32() noexcept;

    uint64_t state_;
    uint64_t inc_;
};

}  // namespace rnd

#endif
