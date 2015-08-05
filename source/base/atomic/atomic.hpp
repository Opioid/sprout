#pragma once

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef min
#undef max
#endif

namespace atomic {

inline void add(volatile float& a, float b) {
#ifdef _WIN32

	union bits { float f; int32_t i; };
	bits old_value;
	bits new_value;

	do {
		old_value.f = a;
		new_value.f = old_value.f + b;
	} while (InterlockedCompareExchange(reinterpret_cast<volatile uint32_t*>(&a),
										new_value.i, old_value.i) != old_value.i);

#else

	float old_value;
	float new_value;

	do {
		old_value = a;
		new_value = old_value + b;
	} while (__sync_val_compare_and_swap(&a, new_value, old_value) != old_value);

#endif
}

}
