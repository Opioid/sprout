#ifndef SU_CORE_RENDERING_SENSOR_FILTER_GAUSSIAN_HPP
#define SU_CORE_RENDERING_SENSOR_FILTER_GAUSSIAN_HPP

#include "sensor_filter.hpp"
#include "base/math/vector3.hpp"
#include "base/math/function/interpolated_function.hpp"

namespace rendering::sensor::filter {

class Gaussian : public Filter {

public:

	Gaussian(float radius, float alpha);

	virtual float radius() const override final;

	virtual float evaluate(float d) const override final;

	virtual float evaluate(float2 p) const override final;

private:

	float gaussian(float d) const;

	float radius_;

	math::Interpolated_function<float> gaussian_;

	float exp_;

	float alpha_;
};

}

#endif
