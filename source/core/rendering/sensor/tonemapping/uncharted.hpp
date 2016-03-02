#pragma once

#include "tonemapper.hpp"

namespace rendering { namespace sensor { namespace tonemapping {

class Uncharted : public Tonemapper {
public:

	Uncharted(math::pfloat3 linear_white, float exposure);

	virtual math::float3 tonemap(math::pfloat3 color) const final override;

private:

	static math::float3 tonemap_function(math::pfloat3 color);

	math::float3 white_factor_;

	float exposure_factor_;
};

}}}
