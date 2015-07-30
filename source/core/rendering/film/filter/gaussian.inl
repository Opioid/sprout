#include "gaussian.hpp"
#include "math/vector.inl"
#include "base/math/function/interpolated_function.inl"
#include <algorithm>

namespace rendering { namespace film { namespace filter {

class Gaussian_functor {
public:

	Gaussian_functor(float radius, float alpha) :
		exp_(std::exp(-alpha * radius * radius)),
		alpha_(alpha) {}

	float operator()(float x) {
		return std::max(0.f, std::exp(-alpha_ * x * x) - exp_);
	}

private:

	float exp_;
	float alpha_;
};

inline Gaussian::Gaussian(float radius, float alpha) :
	gaussian_(0.f, radius, 4, Gaussian_functor(radius, alpha)),
	exp_(std::exp(-alpha * radius * radius)),
	alpha_(alpha) {}

inline float Gaussian::evaluate(math::float2 p) const {
	return gaussian(p.x) * gaussian(p.y);
}

inline float Gaussian::gaussian(float d) const {
	return std::max(0.f, std::exp(-alpha_ * d * d) - exp_);
}

}}}
