#ifndef SU_BASE_MEMORY_BITFIELD_HPP
#define SU_BASE_MEMORY_BITFIELD_HPP

#include <cstdint>

namespace memory {

class Bitfield {
  public:
    Bitfield(uint64_t num_bits);

    ~Bitfield();

    uint64_t num_bytes() const;

    void clear();

    void set(uint64_t index, bool value);

    bool get(uint64_t index) const;

    uint32_t* data() const;

    static uint64_t num_bytes(uint64_t num_bits);

  private:
    uint64_t num_bytes_;

    uint32_t* buffer_;

    static uint32_t constexpr Mask     = 0x80000000;
    static uint32_t constexpr Log2Bits = 5;
    static uint32_t constexpr Bits     = 1 << Log2Bits;
};

}  // namespace memory

#endif
