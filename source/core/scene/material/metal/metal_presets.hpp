#pragma once

#include "base/math/vector.hpp"
#include <string>

namespace scene { namespace material { namespace metal {

void ior_and_absorption(const std::string& name, float3& ior, float3& absorption);

}}}
