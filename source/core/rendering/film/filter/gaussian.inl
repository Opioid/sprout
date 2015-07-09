#include "gaussian.hpp"
#include "math/vector.inl"
#include <algorithm>

namespace rendering { namespace film { namespace filter {

inline Gaussian::Gaussian(math::float2 width, float alpha) :
	alpha_(alpha),
	exp_(math::float2(std::exp(-alpha * width.x * width.x), std::exp(-alpha * width.y * width.y))) {}

inline float Gaussian::evaluate(math::float2 p) const {
	return gaussian(p.x, exp_.x) * gaussian(p.y, exp_.y);
}

inline float Gaussian::gaussian(float d, float exp) const {
	return std::max(0.f, std::exp(-alpha_ * d * d) - exp);
}

}}}
