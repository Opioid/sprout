#pragma once

#include "tonemapper.hpp"

namespace rendering { namespace sensor { namespace tonemapping {

class Filmic : public Tonemapper {
public:

	Filmic(math::pvec3 linear_white);

	virtual math::vec3 tonemap(math::pvec3 color) const final override;

private:

	static math::vec3 tonemap_function(math::pvec3 color);

	math::vec3 linear_white_;
};

}}}
