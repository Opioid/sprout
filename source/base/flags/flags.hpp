#pragma once

namespace flags {

template<typename T>
struct Flags {
	using impl_type = typename std::underlying_type<T>::type;

	inline Flags() : values(impl_type(0)) {}

	inline bool test(T flag) const {
		return static_cast<impl_type>(flag) == (values & static_cast<impl_type>(flag));
	}

	inline void set(T flag) {
		values |= static_cast<impl_type>(flag);
	}

	inline void clear_set(T flag) {
		values = static_cast<impl_type>(flag);
	}

	impl_type values;
};

}
