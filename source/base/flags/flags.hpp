#ifndef SU_BASE_FLAGS_FLAGS_HPP
#define SU_BASE_FLAGS_FLAGS_HPP

#include <type_traits>

namespace flags {

template <typename T>
struct Flags {
    using impl_type = typename std::underlying_type<T>::type;

    Flags() noexcept = default;

    Flags(T flag) : values(static_cast<impl_type>(flag)) {}

    constexpr bool empty() const noexcept {
        return impl_type(0) != values;
    }

    constexpr bool test(T flag) const noexcept {
        return static_cast<impl_type>(flag) == (values & static_cast<impl_type>(flag));
    }

    constexpr bool test_not(T flag) const noexcept {
        return 0 == (values & static_cast<impl_type>(flag));
    }

    constexpr bool test_any(T a, T b) const noexcept {
        return static_cast<impl_type>(a) == (values & static_cast<impl_type>(a)) ||
               static_cast<impl_type>(b) == (values & static_cast<impl_type>(b));
    }

    constexpr bool test_any(T a, T b, T c) const noexcept {
        return static_cast<impl_type>(a) == (values & static_cast<impl_type>(a)) ||
               static_cast<impl_type>(b) == (values & static_cast<impl_type>(b)) ||
               static_cast<impl_type>(c) == (values & static_cast<impl_type>(c));
    }

    constexpr bool operator!=(Flags other) const noexcept {
        return values != other.values;
    }

    void set(T flag) noexcept {
        values |= static_cast<impl_type>(flag);
    }

    void set(T flag, bool value) noexcept {
        if (value) {
            values |= static_cast<impl_type>(flag);
        } else {
            values &= ~static_cast<impl_type>(flag);
        }
    }

    void unset(T flag) noexcept {
        values &= ~static_cast<impl_type>(flag);
    }

    void clear() noexcept {
        values = impl_type(0);
    }

    void clear(T flag) noexcept {
        values = static_cast<impl_type>(flag);
    }

    impl_type values = impl_type(0);
};

}  // namespace flags

#endif
