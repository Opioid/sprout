#ifndef SU_BASE_ATOMIC_ATOMIC_HPP
#define SU_BASE_ATOMIC_ATOMIC_HPP

#include <cstdint>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#undef min
#undef max
#endif

namespace atomic {

static inline void add_assign(uint32_t volatile& a, uint32_t b) noexcept {
    uint32_t old_value;
    uint32_t new_value;

    uint32_t volatile* target = &a;

#ifdef _WIN32

    do {
        old_value = a;
        new_value = old_value + b;
    } while (InterlockedCompareExchange(target, new_value, old_value) != old_value);

#else

    do {
        old_value = a;
        new_value = old_value + b;
    } while (__sync_val_compare_and_swap(target, old_value, new_value) != old_value);

#endif
}

static inline void add_assign(float volatile& a, float b) noexcept {
    union bits {
        float    f;
        uint32_t i;
    };

    bits old_value;
    bits new_value;

    uint32_t volatile* target = reinterpret_cast<uint32_t volatile*>(&a);

#ifdef _WIN32

    do {
        old_value.f = a;
        new_value.f = old_value.f + b;
    } while (InterlockedCompareExchange(target, new_value.i, old_value.i) != old_value.i);

#else

    do {
        old_value.f = a;
        new_value.f = old_value.f + b;
    } while (__sync_val_compare_and_swap(target, old_value.i, new_value.i) != old_value.i);

#endif
}

}  // namespace atomic

#endif
