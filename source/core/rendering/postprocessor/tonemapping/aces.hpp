#pragma once

#include "tonemapper.hpp"

namespace rendering { namespace postprocessor { namespace tonemapping {

class Aces : public Tonemapper {

public:

	Aces(float hdr_max);

private:

	virtual void apply(int32_t begin, int32_t end, uint32_t pass,
					   const image::Float_4& source,
					   image::Float_4& destination) override final;

	static float tonemap_function(float x);

	float normalization_factor_;
};

}}}
