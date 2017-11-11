#ifndef SU_CORE_RENDERING_SENSOR_FILTER_FILTER_HPP
#define SU_CORE_RENDERING_SENSOR_FILTER_FILTER_HPP

#include "base/math/vector3.hpp"

namespace rendering::sensor::filter {

class Filter {

public:

	virtual ~Filter() {}

	virtual float radius() const = 0;

	virtual float evaluate(float d) const = 0;
	virtual float evaluate(float2 p) const = 0;
};

}

#endif
