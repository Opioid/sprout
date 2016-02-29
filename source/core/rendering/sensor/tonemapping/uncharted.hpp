#pragma once

#include "tonemapper.hpp"

namespace rendering { namespace sensor { namespace tonemapping {

class Uncharted : public Tonemapper {
public:

	Uncharted(math::pvec3 linear_white, float exposure);

	virtual math::vec3 tonemap(math::pvec3 color) const final override;

private:

	static math::vec3 tonemap_function(math::pvec3 color);

	math::vec3 white_factor_;

	float exposure_factor_;
};

}}}
