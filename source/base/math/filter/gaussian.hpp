#pragma once

#include <cmath>

namespace math { namespace filter {

class Gaussian_functor {

public:

	Gaussian_functor(float squared_radius, float alpha) :
		exp_(std::exp(-alpha * squared_radius)),
		alpha_(alpha) {}

	float operator()(float squared_d) {
		return std::max(0.f, std::exp(-alpha_ * squared_d) - exp_);
	}

private:

	float exp_;
	float alpha_;
};

}}
