#pragma once

#include "tonemapper.hpp"

namespace rendering { namespace postprocessor { namespace tonemapping {

class Generic : public Tonemapper {

public:

	Generic(float contrast, float shoulder, float mid_in, float mid_out, float hdr_max);

private:

	virtual void apply(int32_t begin, int32_t end, uint32_t pass,
					   const image::Float_4& source,
					   image::Float_4& destination) final override;

	float tonemap_function(float x) const;

	float a_;
	float b_;
	float c_;
	float d_;

	float hdr_max_;
};

}}}
