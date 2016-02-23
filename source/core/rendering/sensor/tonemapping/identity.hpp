#pragma once

#include "tonemapper.hpp"

namespace rendering { namespace sensor { namespace tonemapping {

class Identity : public Tonemapper {
public:

	virtual math::vec3 tonemap(math::pvec3 color) const final override;
};

}}}
