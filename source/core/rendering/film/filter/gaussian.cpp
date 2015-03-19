#include "gaussian.hpp"
#include "math/vector.inl"
#include <algorithm>

namespace rendering { namespace film { namespace filter {

Gaussian::Gaussian(const math::float2& width, float alpha) :
	alpha_(alpha),
	exp_(math::float2(std::exp(-alpha * width.x * width.x), std::exp(-alpha * width.y * width.y))) {}

float Gaussian::evaluate(const math::float2& p) const {
	return gaussian(p.x, exp_.x) * gaussian(p.y, exp_.y);
}

float Gaussian::gaussian(float d, float exp) const {
	return std::max(0.f, std::exp(-alpha_ * d * d) - exp);
}

}}}
