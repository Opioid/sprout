#ifndef SU_BASE_MEMORY_BITFIELD_HPP
#define SU_BASE_MEMORY_BITFIELD_HPP

#include <cstdint>

namespace memory {

class Bitfield {
  public:
    Bitfield(size_t num_bits) noexcept;

    ~Bitfield() noexcept;

    size_t num_bytes() const noexcept;

    void clear() noexcept;

    void set(size_t index, bool value) noexcept;

    bool get(size_t index) const noexcept;

    uint32_t* data() const noexcept;

    static size_t num_bytes(size_t num_bits) noexcept;

  private:
    uint64_t num_bytes_;

    uint32_t* buffer_;

    static uint32_t constexpr Mask     = 0x80000000;
    static uint32_t constexpr Log2Bits = 5;
    static uint32_t constexpr Bits     = 1 << Log2Bits;
};

}  // namespace memory

#endif
