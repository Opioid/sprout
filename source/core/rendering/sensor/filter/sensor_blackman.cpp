#include "sensor_blackman.hpp"
#include "base/math/interpolated_function_1d.inl"
#include "base/math/math.hpp"
#include "math/vector2.inl"

namespace rendering::sensor::filter {

struct Functor {
    Functor(float r) : r_(r) {}

    float operator()(float x) const {
        float const a0 = 0.35875f;
        float const a1 = 0.48829f;
        float const a2 = 0.14128f;
        float const a3 = 0.01168f;

        x = ((Pi * (x + r_)) / r_);

        return a0 - a1 * std::cos(x) + a2 * std::cos(2.f * x) - a3 * std::cos(3.f * x);
    }

    float r_;
};

Blackman::Blackman(float radius) : func_(0.f, radius, Functor(radius)) {
    float const i = integral(64, radius);
    func_.scale(1.f / i);
}

Blackman::Blackman(Blackman&& other) noexcept : func_(std::move(other.func_)) {}

Blackman::~Blackman() = default;

float Blackman::evaluate(float d) const {
    return func_(std::abs(d));
}

float Blackman::evaluate(float2 p) const {
    return func_(std::abs(p[0])) * func_(std::abs(p[1]));
}

float Blackman::integral(uint32_t num_samples, float radius) const {
    // The function is symmetric; so integrate one half, then double the result
    float const interval = radius / float(num_samples);

    float s = 0.5f * interval;

    float sum = 0.f;

    for (uint32_t i = 0; i < num_samples; ++i, s += interval) {
        float const v = evaluate(s);

        float const a = v * interval;

        sum += a;
    }

    return sum + sum;
}

}  // namespace rendering::sensor::filter
