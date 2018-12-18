#include "sensor_gaussian.hpp"
#include <algorithm>
#include "base/math/filter/gaussian.hpp"
#include "base/math/interpolated_function.inl"
#include "math/vector3.inl"

namespace rendering::sensor::filter {

Gaussian::Gaussian(float radius, float alpha)
    : radius_(radius),
      gaussian_(0.f, radius * radius, 256, math::filter::Gaussian_functor(radius * radius, alpha))
/*      exp_(std::exp(-alpha * radius * radius)),
      alpha_(alpha)*/
{}

float Gaussian::radius() const {
    return radius_;
}

float Gaussian::evaluate(float d) const {
    return gaussian_(d * d);

    //	return gaussian(d);
}

float Gaussian::evaluate(float2 p) const {
    return gaussian_(p[0] * p[0]) * gaussian_(p[1] * p[1]);

    //	return gaussian(p.x) * gaussian(p.y);
}

// inline float Gaussian::gaussian(float d) const {
//    return std::max(0.f, std::exp(-alpha_ * d * d) - exp_);
//}

}  // namespace rendering::sensor::filter
