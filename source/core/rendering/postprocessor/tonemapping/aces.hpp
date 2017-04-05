#pragma once

#include "tonemapper.hpp"

namespace rendering { namespace postprocessor { namespace tonemapping {

class Aces : public Tonemapper {

public:

	Aces(float hdr_max);

	virtual float3 tonemap(const float3& color) const final override;

private:

	static float tonemap_function(float x);

	float normalization_factor_;
};

}}}
