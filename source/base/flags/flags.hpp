#pragma once

namespace flags {

template<typename T>
struct Flags {
	using impl_type = typename std::underlying_type<T>::type;

	Flags() : values(impl_type(0)) {}

	bool test(T flag) const {
		return static_cast<impl_type>(flag) == (values & static_cast<impl_type>(flag));
	}

	void set(T flag) {
		values |= static_cast<impl_type>(flag);
	}

	void clear_set(T flag) {
		values = static_cast<impl_type>(flag);
	}

	impl_type values;
};

}
