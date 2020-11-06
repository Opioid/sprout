#ifndef SU_BASE_FLAGS_FLAGS_HPP
#define SU_BASE_FLAGS_FLAGS_HPP

#include <type_traits>

namespace flags {

template <typename T>
struct Flags {
    using impl_type = typename std::underlying_type<T>::type;

    Flags() : values(impl_type(0)) {}

    Flags(T a) : values(impl_type(a)) {}

    Flags(T a, T b) : values(impl_type(a) | impl_type(b)) {}

    Flags(T a, T b, T c) : values(impl_type(a) | impl_type(b) | impl_type(c)) {}

    constexpr bool is(T flag) const {
        return 0 != (values & impl_type(flag));
    }

    constexpr bool no(T flag) const {
        return 0 == (values & impl_type(flag));
    }

    constexpr bool any(T a, T b) const {
        return 0 != (values & (impl_type(a) | impl_type(b)));
    }

    constexpr bool any(T a, T b, T c) const {
        return 0 != (values & (impl_type(a) | impl_type(b) | impl_type(c)));
    }

    constexpr bool operator!=(Flags other) const {
        return values != other.values;
    }

    void set(T flag) {
        values |= impl_type(flag);
    }

    void set(T flag, bool value) {
        if (value) {
            values |= impl_type(flag);
        } else {
            values &= ~impl_type(flag);
        }
    }

    void and_set(T flag, bool value) {
        if (is(flag) & (!value)) {
            values &= ~impl_type(flag);
        }
    }

    void or_set(T flag, bool value) {
        if (value) {
            values |= impl_type(flag);
        }
    }

    void unset(T flag) {
        values &= ~impl_type(flag);
    }

    void clear() {
        values = impl_type(0);
    }

    void clear(T flag) {
        values = impl_type(flag);
    }

    impl_type values;
};

}  // namespace flags

#endif
