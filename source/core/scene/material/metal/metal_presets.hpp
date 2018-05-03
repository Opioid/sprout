#pragma once

#include "base/math/vector3.hpp"
#include <string>

namespace scene::material::metal {

void ior_and_absorption(std::string const& name, float3& ior, float3& absorption);

}
