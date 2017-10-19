#pragma once

#include <type_traits>

namespace flags {

template<typename T>
struct Flags {
	using impl_type = typename std::underlying_type<T>::type;

	Flags() = default;

	bool test(T flag) const {
		return static_cast<impl_type>(flag) == (values & static_cast<impl_type>(flag));
	}

	bool test_any(T a, T b) const {
		return static_cast<impl_type>(a) == (values & static_cast<impl_type>(a))
			|| static_cast<impl_type>(b) == (values & static_cast<impl_type>(b));
	}

	bool test_any(T a, T b, T c) const {
		return static_cast<impl_type>(a) == (values & static_cast<impl_type>(a))
			|| static_cast<impl_type>(b) == (values & static_cast<impl_type>(b))
			|| static_cast<impl_type>(c) == (values & static_cast<impl_type>(c));
	}

	void set(T flag) {
		values |= static_cast<impl_type>(flag);
	}

	void unset(T flag) {
		values &= ~static_cast<impl_type>(flag);
	}

	void set(T flag, bool value) {
		if (value) {
			values |= static_cast<impl_type>(flag);
		} else {
			values &= ~static_cast<impl_type>(flag);
		}
	}

	void clear_set(T flag) {
		values = static_cast<impl_type>(flag);
	}

	void clear() {
		values = impl_type(0);
	}

	bool operator!=(Flags other) const {
		return values != other.values;
	}

	impl_type values = impl_type(0);
};

}
