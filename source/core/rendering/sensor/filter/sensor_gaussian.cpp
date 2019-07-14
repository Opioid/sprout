#include "sensor_gaussian.hpp"
#include "base/math/filter/gaussian.hpp"
#include "base/math/interpolated_function.inl"
#include "math/vector3.inl"

#include <iostream>
#include "base/math/print.hpp"

namespace rendering::sensor::filter {

Gaussian::Gaussian(float radius, float alpha) noexcept
    : radius_(radius),
      gaussian_(0.f, radius * radius, 16, math::filter::Gaussian_functor(radius * radius, alpha))
/*      exp_(std::exp(-alpha * radius * radius)),
      alpha_(alpha)*/
{
    std::cout << integral() << std::endl;
}

Gaussian::Gaussian(Gaussian&& other) noexcept
    : radius_(other.radius_), gaussian_(std::move(other.gaussian_)) {}

float Gaussian::radius() const noexcept {
    return radius_;
}

float Gaussian::evaluate(float d) const noexcept {
    return (1.f / std::sqrt(0.8f)) * gaussian_(d * d);

    //	return gaussian(d);
}

float Gaussian::evaluate(float2 p) const noexcept {
//    return (1.f / std::sqrt(0.382f)) * gaussian_(p[0] * p[0]) * (1.f / std::sqrt(0.382f)) * gaussian_(p[1] * p[1]);

    return gaussian_(p[0] * p[0]) * gaussian_(p[1] * p[1]);

    //	return gaussian(p.x) * gaussian(p.y);
}

float Gaussian::integral() const noexcept {
    uint32_t const num_samples = 4096;

    float2 v(-radius_, -radius_);

    float const interval = (2.f * radius_) / static_cast<float>(num_samples);

    float sum = 0.f;

    uint32_t actual_samples = 0;

    for (uint32_t y = 0; y <= num_samples; ++y) {
        for (uint32_t x = 0; x <= num_samples; ++x) {
            if (length(v) <= radius_) {
                sum += evaluate(v);

                ++actual_samples;
            }

            v[0] += interval;
        }

        v[0] = -radius_;
        v[1] += interval;
    }

    return sum / static_cast<float>(actual_samples);
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
