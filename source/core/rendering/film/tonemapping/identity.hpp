#pragma once

#include "tonemapper.hpp"

namespace rendering { namespace film { namespace tonemapping {

class Identity : public Tonemapper {
public:

	virtual math::float3 tonemap(const math::float3& color) const;
};

}}}
