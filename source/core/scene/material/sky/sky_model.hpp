#pragma once

#include "base/math/vector.hpp"

namespace scene { namespace material { namespace sky {

class Model {
public:

	void set_ground_albedo(math::pfloat3 albedo);

	void set_turbidity(float turbidity);


private:

	math::float3 ground_albedo_;

	float turbidity_;

};

}}}
