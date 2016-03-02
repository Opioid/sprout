#pragma once

#include "tonemapper.hpp"

namespace rendering { namespace sensor { namespace tonemapping {

class Identity : public Tonemapper {
public:

	virtual math::float3 tonemap(math::pfloat3 color) const final override;
};

}}}
