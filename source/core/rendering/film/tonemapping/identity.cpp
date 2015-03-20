#pragma once

#include "identity.hpp"

namespace rendering { namespace film { namespace tonemapping {

math::float3 Identity::tonemap(const math::float3& color) const {
	return color;
}

}}}
