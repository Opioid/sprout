#pragma once

#include "base/math/vector.hpp"

namespace rendering { namespace film { namespace filter {

class Gaussian {
public:

	Gaussian(math::float2 width, float alpha);

	float evaluate(math::float2 p) const;

private:

	float gaussian(float d, float exp) const;

	float alpha_;

	math::float2 exp_;
};

}}}
