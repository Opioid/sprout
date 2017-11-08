#ifndef SU_BASE_ATOMIC_ATOMIC_HPP
#define SU_BASE_ATOMIC_ATOMIC_HPP

#include <cstdint>

#ifdef _WIN32
#	define WIN32_LEAN_AND_MEAN
#	include <windows.h>
#	undef min
#	undef max
#endif

namespace atomic {

static inline void add_assign(volatile float& a, float b) noexcept {
	union bits { float f; uint32_t i; };

	bits old_value;
	bits new_value;

	volatile uint32_t* target = reinterpret_cast<volatile uint32_t*>(&a);

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

}

#endif
