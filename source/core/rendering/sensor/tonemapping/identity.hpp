#pragma once

#include "tonemapper.hpp"

namespace rendering { namespace sensor { namespace tonemapping {

class Identity : public Tonemapper {
public:

	virtual float3 tonemap(float3_p color) const final override;
};

}}}
