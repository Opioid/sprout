#include "sensor_gaussian.hpp"
#include "base/math/filter/gaussian.hpp"
#include "base/math/interpolated_function_1d.inl"
#include "math/vector3.inl"

namespace rendering::sensor::filter {

Gaussian::Gaussian(float radius, float alpha) noexcept
    : radius_(radius),
      gaussian_(0.f, radius * radius, 16, math::filter::Gaussian_functor(radius * radius, alpha))
/*      exp_(std::exp(-alpha * radius * radius)),
      alpha_(alpha)*/
{
    float const i = integral(64);
    gaussian_.scale(1.f / i);
}

Gaussian::Gaussian(Gaussian&& other) noexcept
    : radius_(other.radius_), gaussian_(std::move(other.gaussian_)) {}

float Gaussian::radius() const noexcept {
    return radius_;
}

float Gaussian::evaluate(float d) const noexcept {
    return gaussian_(d * d);

    //	return gaussian(d);
}

float Gaussian::evaluate(float2 p) const noexcept {
    return gaussian_(p[0] * p[0]) * gaussian_(p[1] * p[1]);

    //	return gaussian(p.x) * gaussian(p.y);
}

float Gaussian::integral(uint32_t num_samples) const noexcept {
    float const interval = (2.f * radius_) / float(num_samples);

    float s = -radius_ + 0.5f * interval;

    float sum = 0.f;

    for (uint32_t i = 0; i < num_samples; ++i, s += interval) {
        float const v = evaluate(s);

        float const a = v * interval;

        sum += a;
    }

    return sum;
}

// inline float Gaussian::gaussian(float d) const {
//    return std::max(0.f, std::exp(-alpha_ * d * d) - exp_);
//}

/*
Gaussian_1::Gaussian_1(float radius, float alpha)
    : radius_(radius),
      gaussian_(0.f, radius, 16, math::filter::Gaussian_functor_1(radius * radius, alpha))
{}

float Gaussian_1::radius() const {
    return radius_;
}

float Gaussian_1::evaluate(float d) const {
    return gaussian_(std::abs(d));
}

float Gaussian_1::evaluate(float2 p) const {
    return gaussian_(std::abs(p[0])) * gaussian_(std::abs(p[1]));
}
*/

}  // namespace rendering::sensor::filter
