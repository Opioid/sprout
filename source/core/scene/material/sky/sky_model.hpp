#pragma once

#include "base/math/vector.hpp"

struct ArHosekSkyModelState;

namespace scene { namespace material { namespace sky {

class Model {
public:

	~Model();

	void init();

	void set_ground_albedo(math::pfloat3 albedo);
	void set_turbidity(float turbidity);

	math::float3 evaluate(math::pfloat3 wi) const;

private:

	math::float3 ground_albedo_;

	float turbidity_;

	ArHosekSkyModelState* skymodel_states_[3];
};

}}}
