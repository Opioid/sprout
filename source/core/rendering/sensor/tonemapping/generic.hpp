#pragma once

#include "tonemapper.hpp"

namespace rendering { namespace sensor { namespace tonemapping {

class Generic : public Tonemapper {

public:

	Generic(float contrast, float shoulder, float mid_in, float mid_out, float hdr_max,
			float exposure);

	virtual float3 tonemap(float3_p color) const final override;

private:

	float3 tonemap_function(float3_p color) const;
	float  tonemap_function(float x) const;

	float a_;
	float b_;
	float c_;
	float d_;

	float exposure_factor_;
};

}}}
