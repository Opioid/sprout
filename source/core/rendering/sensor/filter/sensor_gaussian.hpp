#pragma once

#include "sensor_filter.hpp"
#include "base/math/vector.hpp"
#include "base/math/function/interpolated_function.hpp"

namespace rendering { namespace sensor { namespace filter {

class Gaussian : public Filter {

public:

	Gaussian(float radius, float alpha);

	virtual float radius() const final override;

	virtual float evaluate(float2 p) const final override;

private:

	float gaussian(float d) const;

	float radius_;

	math::Interpolated_function<float> gaussian_;

	float exp_;

	float alpha_;
};

}}}
