#pragma once

#include "tonemapper.hpp"

namespace rendering { namespace sensor { namespace tonemapping {

class Uncharted : public Tonemapper {
public:

	Uncharted(float3_p linear_white, float exposure);

	virtual float3 tonemap(float3_p color) const final override;

private:

	static float3 tonemap_function(float3_p color);

	float3 white_factor_;

	float exposure_factor_;
};

}}}
