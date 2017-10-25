#pragma once

#include <type_traits>

namespace flags {

template<typename T>
struct Flags {
	using impl_type = typename std::underlying_type<T>::type;

	Flags() = default;

	constexpr bool test(T flag) const {
		return static_cast<impl_type>(flag) == (values & static_cast<impl_type>(flag));
	}

	constexpr bool test_any(T a, T b) const {
		return static_cast<impl_type>(a) == (values & static_cast<impl_type>(a))
			|| static_cast<impl_type>(b) == (values & static_cast<impl_type>(b));
	}

	constexpr bool test_any(T a, T b, T c) const {
		return static_cast<impl_type>(a) == (values & static_cast<impl_type>(a))
			|| static_cast<impl_type>(b) == (values & static_cast<impl_type>(b))
			|| static_cast<impl_type>(c) == (values & static_cast<impl_type>(c));
	}

	constexpr bool operator!=(Flags other) const {
		return values != other.values;
	}

	void set(T flag) {
		values |= static_cast<impl_type>(flag);
	}

	void set(T flag, bool value) {
		if (value) {
			values |= static_cast<impl_type>(flag);
		} else {
			values &= ~static_cast<impl_type>(flag);
		}
	}

	void unset(T flag) {
		values &= ~static_cast<impl_type>(flag);
	}

	void clear() {
		values = impl_type(0);
	}

	void clear(T flag) {
		values = static_cast<impl_type>(flag);
	}

	impl_type values = impl_type(0);
};

}
