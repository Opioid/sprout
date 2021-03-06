#ifndef SU_BASE_MEMORY_BITFIELD_INL
#define SU_BASE_MEMORY_BITFIELD_INL

#include "bitfield.hpp"

#include <cstring>

namespace memory {

inline Bitfield::Bitfield(uint64_t num_bits)
    : num_bytes_(num_bytes(num_bits)),
      buffer_(new uint32_t[num_bytes(num_bits) / sizeof(uint32_t)]) {}

inline Bitfield::~Bitfield() {
    delete[] buffer_;
}

inline uint64_t Bitfield::num_bytes() const {
    return num_bytes_;
}

inline void Bitfield::clear() {
    std::memset(buffer_, 0, num_bytes_);
}

inline void Bitfield::set(uint64_t index, bool value) {
    uint32_t const mask = Mask >> (index % Bits);

    if (value) {
        buffer_[index >> Log2Bits] |= mask;
    } else {
        buffer_[index >> Log2Bits] &= ~mask;
    }
}

inline bool Bitfield::get(uint64_t index) const {
    uint32_t const mask = Mask >> (index % Bits);

    return (buffer_[index >> Log2Bits] & mask) != 0;
}

inline uint32_t* Bitfield::data() const {
    return buffer_;
}

inline uint64_t Bitfield::num_bytes(uint64_t num_bits) {
    uint64_t const chunks = num_bits / Bits;
    return ((num_bits % Bits == 0) ? chunks : chunks + 1) * sizeof(uint32_t);
}

}  // namespace memory

#endif
