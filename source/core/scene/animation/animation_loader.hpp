#pragma once

#include "base/math/transformation.hpp"
#include "base/json/rapidjson_types.hpp"
#include <memory>

namespace scene { namespace animation {

class Animation;

std::shared_ptr<animation::Animation> load(const rapidjson::Value& animation_value,
										   const math::transformation& default_transformation);

}}
