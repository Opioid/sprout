#ifndef SU_SCENE_EXTENSION_PROVIDER_HPP
#define SU_SCENE_EXTENSION_PROVIDER_HPP

#include "base/json/json_types.hpp"

#include <cstdint>

namespace resource {
class Manager;
}

namespace scene {

class Scene;

class Extension_provider {
  public:
    using Resources = resource::Manager;

    virtual ~Extension_provider() = default;

    virtual uint32_t create_extension(json::Value const& extension_value, Scene& scene,
                                      Resources& resources) = 0;
};

}  // namespace scene

#endif
