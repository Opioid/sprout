#pragma once

#include "base/json/json_types.hpp"

namespace resource {
class Manager;
}

namespace scene {

class Scene;

namespace entity {

class Entity;

class Extension_provider {
  public:
    virtual Entity* create_extension(json::Value const& extension_value, Scene& scene,
                                     resource::Manager& manager) = 0;
};

}  // namespace entity
}  // namespace scene
