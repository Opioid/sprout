#ifndef SU_CORE_SCENE_ANIMATION_LOADER_HPP
#define SU_CORE_SCENE_ANIMATION_LOADER_HPP

#include "base/json/json_types.hpp"

#include <cstdint>

namespace math {
struct Transformation;
}

namespace scene {

class Scene;

namespace animation {

bool load(json::Value const& animation_value, math::Transformation const& default_trafo,
          uint32_t entity, Scene& scene);

}  // namespace animation
}  // namespace scene

#endif
