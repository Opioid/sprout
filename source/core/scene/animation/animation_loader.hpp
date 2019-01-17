#ifndef SU_CORE_SCENE_ANIMATION_LOADER_HPP
#define SU_CORE_SCENE_ANIMATION_LOADER_HPP

#include <memory>
#include "base/json/json_types.hpp"
#include "base/math/transformation.hpp"

namespace scene::animation {

class Animation;

animation::Animation* load(json::Value const&          animation_value,
                           math::Transformation const& default_transformation);

}  // namespace scene::animation

#endif
