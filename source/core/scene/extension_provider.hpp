#ifndef SU_SCENE_EXTENSION_PROVIDER_HPP
#define SU_SCENE_EXTENSION_PROVIDER_HPP

#include <string>
#include "base/json/json_types.hpp"

namespace resource {
class Manager;
}

namespace scene {

class Scene;

namespace entity {
struct Entity_ref;
}

class Extension_provider {
  public:
    using Entity_ref = entity::Entity_ref;

    virtual ~Extension_provider() noexcept {}

    virtual Entity_ref create_extension(json::Value const& extension_value, std::string const& name,
                                        Scene& scene, resource::Manager& manager) noexcept = 0;
};

}  // namespace scene

#endif
