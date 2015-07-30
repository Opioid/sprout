#include "gaussian.hpp"
#include "math/vector.inl"
#include "base/math/function/interpolated_function.inl"
#include <algorithm>

#include <iostream>

namespace rendering { namespace film { namespace filter {

class Gaussian_functor {
public:

	Gaussian_functor(float radius_square, float alpha) :
		exp_(std::exp(-alpha * radius_square)),
		alpha_(alpha) {}

	float operator()(float x) {
		return std::max(0.f, std::exp(-alpha_ * x) - exp_);
	}

private:

	float exp_;
	float alpha_;
};

inline Gaussian::Gaussian(float radius, float alpha) :
	gaussian_(0.f, radius * radius, 4, Gaussian_functor(radius * radius, alpha)),
	exp_(std::exp(-alpha * radius * radius)),
	alpha_(alpha) {}

inline float Gaussian::evaluate(math::float2 p) const {

//	float calced = gaussian(p.x);
//	float cached = gaussian_.f(p.x * p.x);

//	std::cout << std::abs(calced - cached) << std::endl;

	return gaussian_.f(p.x * p.x) * gaussian_.f(p.y * p.y);

//	return gaussian(p.x) * gaussian(p.y);
}

inline float Gaussian::gaussian(float d) const {
	return std::max(0.f, std::exp(-alpha_ * d * d) - exp_);
}

}}}
