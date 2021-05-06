#include "sensor_gaussian.hpp"
#include "base/math/interpolated_function_1d.inl"
#include "math/vector2.inl"

namespace rendering::sensor::filter {

class Functor {
  public:
    Functor(float squared_radius, float alpha)
        : exp_(std::exp(-alpha * squared_radius)), alpha_(alpha) {}

    float operator()(float squared_d) const {
        return std::max(0.f, std::exp(-alpha_ * squared_d) - exp_);
    }

  private:
    float exp_;
    float alpha_;
};

/*
class Gaussian_functor_1 {
  public:
    Gaussian_functor_1(float squared_radius, float alpha)
        : exp_(std::exp(-alpha * squared_radius)), alpha_(alpha) {}

    float operator()(float d) const {
        return std::max(0.f, std::exp(-alpha_ * d * d) - exp_);
    }

  private:
    float exp_;
    float alpha_;
};
*/

Gaussian::Gaussian(float radius, float alpha)
    : gaussian_(0.f, radius * radius, Functor(radius * radius, alpha)) {
    float const i = integral(64, radius);
    gaussian_.scale(1.f / i);
}

Gaussian::Gaussian(Gaussian&& other) noexcept : gaussian_(std::move(other.gaussian_)) {}

Gaussian::~Gaussian() = default;

float Gaussian::evaluate(float d) const {
    return gaussian_(d * d);
}

float Gaussian::evaluate(float2 p) const {
    return gaussian_(p[0] * p[0]) * gaussian_(p[1] * p[1]);
}

float Gaussian::integral(uint32_t num_samples, float radius) const {
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

/*
Gaussian_1::Gaussian_1(float radius, float alpha)
    : radius_(radius),
      gaussian_(0.f, radius, 16, math::filter::Gaussian_functor_1(radius * radius, alpha))
{
    float const i = integral(64);
    gaussian_.scale(1.f / i);
}

Gaussian_1::~Gaussian_1() = default;

float Gaussian_1::radius() const {
    return radius_;
}

float Gaussian_1::evaluate(float d) const {
    return gaussian_(std::abs(d));
}

float Gaussian_1::evaluate(float2 p) const {
    return gaussian_(std::abs(p[0])) * gaussian_(std::abs(p[1]));
}

float Gaussian_1::integral(uint32_t num_samples) const {
    // The function is symmetric; so integrate one half, then double the result
    float const interval = radius_ / float(num_samples);

    float s = 0.5f * interval;

    float sum = 0.f;

    for (uint32_t i = 0; i < num_samples; ++i, s += interval) {
        float const v = evaluate(s);

        float const a = v * interval;

        sum += a;
    }

    return sum + sum;
}

Reference_gaussian::Reference_gaussian(float radius, float alpha)
    : radius_(radius),
      exp_(std::exp(-alpha * radius * radius)),
            alpha_(alpha),
            integral_(1.f)
{
    integral_ = integral(64);
}

float Reference_gaussian::radius() const {
    return radius_;
}

float Reference_gaussian::evaluate(float d) const {
    return std::max(0.f, (std::exp(-alpha_ * d * d) - exp_) / integral_);
}

float Reference_gaussian::evaluate(float2 p) const {
    return evaluate(p[0]) * evaluate(p[1]);
}

float Reference_gaussian::integral(uint32_t num_samples) const {
    // The function is symmetric; so integrate one half, then double the result
    float const interval = radius_ / float(num_samples);

    float s = 0.5f * interval;

    float sum = 0.f;

    for (uint32_t i = 0; i < num_samples; ++i, s += interval) {
        float const v = evaluate(s);

        float const a = v * interval;

        sum += a;
    }

    return sum + sum;
}
*/
}  // namespace rendering::sensor::filter
