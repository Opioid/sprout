#ifndef SU_CORE_SCENE_ANIMATION_LOADER_HPP
#define SU_CORE_SCENE_ANIMATION_LOADER_HPP

#include "base/math/transformation.hpp"
#include "base/json/json_types.hpp"
#include <memory>

namespace scene::animation {

class Animation;

std::shared_ptr<animation::Animation> load(const json::Value& animation_value,
										   const math::Transformation& default_transformation);

}

#endif
