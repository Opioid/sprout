#pragma once

#include "tonemapper.hpp"

namespace rendering { namespace postprocessor { namespace tonemapping {

class Uncharted : public Tonemapper {

public:

	Uncharted(float hdr_max);

	virtual float3 tonemap(float3_p color) const final override;

private:

	static float tonemap_function(float x);

	float normalization_factor_;
};

}}}
