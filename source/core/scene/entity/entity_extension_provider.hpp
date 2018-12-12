#ifndef SU_SCENE_ENTITY_EXTENSION_PROVIDER_HPP
#define SU_SCENE_ENTITY_EXTENSION_PROVIDER_HPP

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
    virtual ~Extension_provider() noexcept {}

    virtual Entity* create_extension(json::Value const& extension_value, Scene& scene,
                                     resource::Manager& manager) noexcept = 0;
};

}  // namespace entity
}  // namespace scene

#endif
