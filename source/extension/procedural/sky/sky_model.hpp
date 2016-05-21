#pragma once

#include "base/math/vector.hpp"

struct ArHosekSkyModelState;

namespace procedural { namespace sky {

class Model {

public:

	Model();
	~Model();

	void init();

	void set_sun_direction(math::pfloat3 direction);
	void set_ground_albedo(math::pfloat3 albedo);
	void set_turbidity(float turbidity);

	math::float3 evaluate(math::pfloat3 wi) const;

private:

	void release();

	math::float3 sun_direction_;
	math::float3 ground_albedo_;

	float turbidity_;

	ArHosekSkyModelState* skymodel_states_[3];

	static const math::float3 zenith_;
};

}}
