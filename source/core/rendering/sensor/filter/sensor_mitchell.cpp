#include "sensor_mitchell.hpp"
#include <algorithm>
#include "base/math/interpolated_function_1d.inl"
#include "math/vector3.inl"

namespace rendering::sensor::filter {

struct Functor {
    Functor(float b, float c) noexcept : b_(b), c_(c) {}

    float operator()(float x) const noexcept {
        x *= 2.f;

        float const xx = x * x;

        if (x > 1.f) {
            return ((-b_ - 6.f * c_) * xx * x + (6.f * b_ + 30.f * c_) * xx +
                    (-12.f * b_ - 48.f * c_) * x + (8.f * b_ + 24.f * c_)) *
                   (1.f / 6.f);
        } else {
            return ((12.f - 9.f * b_ - 6.f * c_) * xx * x + (-18.f + 12.f * b_ + 6.f * c_) * xx +
                    (6.f - 2.f * b_)) *
                   (1.f / 6.f);
        }
    }

    float b_;
    float c_;
};

Mitchell::Mitchell(float radius, float b, float c) noexcept
    : radius_(radius),
      radius_inv_(1.f / radius),
      mitchell_(0.f, 1.f, 16, Functor(b, c)),
      b_(b),
      c_(c) {}

Mitchell::Mitchell(Mitchell&& other) noexcept
    : radius_(other.radius_),
      radius_inv_(other.radius_inv_),
      mitchell_(std::move(other.mitchell_)) {}

float Mitchell::radius() const noexcept {
    return radius_;
}

float Mitchell::evaluate(float d) const noexcept {
    return mitchell(d * radius_inv_);
}

float Mitchell::evaluate(float2 p) const noexcept {
    return mitchell(p[0] * radius_inv_) * mitchell(p[1] * radius_inv_);
}

float Mitchell::mitchell(float x) const noexcept {
    x = std::abs(2.f * x);

    float const xx = x * x;

    float const b = b_;
    float const c = c_;

    if (x > 1.f) {
        return ((-b - 6.f * c) * xx * x + (6.f * b + 30.f * c) * xx + (-12.f * b - 48.f * c) * x +
                (8.f * b + 24.f * c)) *
               (1.f / 6.f);
    } else {
        return ((12.f - 9.f * b - 6.f * c) * xx * x + (-18.f + 12.f * b + 6.f * c) * xx +
                (6.f - 2.f * b)) *
               (1.f / 6.f);
    }

    //    x = std::abs(x);

    //    if (x >= 1.f) {
    //        return 0.f;
    //    }

    //    return mitchell_(x);
}

}  // namespace rendering::sensor::filter
