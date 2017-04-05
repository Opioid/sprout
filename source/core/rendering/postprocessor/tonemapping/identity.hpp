#pragma once

#include "tonemapper.hpp"

namespace rendering { namespace postprocessor { namespace tonemapping {

class Identity : public Tonemapper {
public:

	virtual float3 tonemap(const float3& color) const final override;
};

}}}
