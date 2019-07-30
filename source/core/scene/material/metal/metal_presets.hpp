#ifndef SU_CORE_SCENE_MATERIAL_METAL_PRESETS_HPP
#define SU_CORE_SCENE_MATERIAL_METAL_PRESETS_HPP

#include <string>
#include "base/math/vector3.hpp"

namespace scene::material::metal {

void ior_and_absorption(std::string const& name, float3& ior, float3& absorption);
}

#endif
