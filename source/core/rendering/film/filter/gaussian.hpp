#pragma once

#include "filter.hpp"

namespace rendering { namespace film { namespace filter {

class Gaussian : public Filter {
public:

	Gaussian(const math::float2& width, float alpha);

	virtual float evaluate(const math::float2& p) const;

private:

	float gaussian(float d, float exp) const;

	float alpha_;

	math::float2 exp_;
};

}}}
