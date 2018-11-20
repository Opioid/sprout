#ifndef SU_BASE_MEMORY_BITFIELD_INL
#define SU_BASE_MEMORY_BITFIELD_INL

#include "bitfield.hpp"
#include <cstring>

namespace memory {

inline Bitfield::Bitfield(size_t num_bits) noexcept
    : num_bytes_(num_bytes(num_bits)),
      buffer_(new uint32_t[num_bytes(num_bits) / sizeof(uint32_t)]) {}

inline Bitfield::~Bitfield() noexcept {
    delete[] buffer_;
}

inline size_t Bitfield::num_bytes() const noexcept {
    return num_bytes_;
}

inline void Bitfield::clear() noexcept {
    std::memset(buffer_, 0, num_bytes_);
}

inline void Bitfield::set(size_t index, bool value) noexcept {
    uint32_t const mask = Mask >> (index % Bits);

    if (value) {
        buffer_[index >> Log2Bits] |= mask;
    } else {
        buffer_[index >> Log2Bits] &= ~mask;
    }
}

inline bool Bitfield::get(size_t index) const noexcept {
    uint32_t const mask = Mask >> (index % Bits);

    return (buffer_[index >> Log2Bits] & mask) != 0;
}

inline size_t Bitfield::num_bytes(size_t num_bits) noexcept {
    size_t const chunks = num_bits / Bits;
    return ((num_bits % Bits == 0) ? chunks : chunks + 1) * sizeof(uint32_t);
}

}  // namespace memory

#endif
