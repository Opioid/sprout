#pragma once

#include "base/json/rapidjson_types.hpp"
#include <memory>

namespace scene { namespace animation {

class Animation;

std::shared_ptr<animation::Animation> load(const rapidjson::Value& animation_value);

}}
