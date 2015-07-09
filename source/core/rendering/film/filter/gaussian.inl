#include "gaussian.hpp"
#include "math/vector.inl"
#include <algorithm>

namespace rendering { namespace film { namespace filter {

inline Gaussian::Gaussian(float radius, float alpha) :
	exp_(std::exp(-alpha * radius * radius)),
	alpha_(alpha) {}

inline float Gaussian::evaluate(math::float2 p) const {
	return gaussian(p.x) * gaussian(p.y);
}

inline float Gaussian::gaussian(float d) const {
	return std::max(0.f, std::exp(-alpha_ * d * d) - exp_);
}

}}}
