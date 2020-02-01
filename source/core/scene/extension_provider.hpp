#ifndef SU_SCENE_EXTENSION_PROVIDER_HPP
#define SU_SCENE_EXTENSION_PROVIDER_HPP

#include <string>
#include "base/json/json_types.hpp"

namespace resource {
class Manager;
}

namespace scene {

class Scene;

namespace prop {
struct Prop_ptr;
}

class Extension_provider {
  public:
    using Prop_ptr  = prop::Prop_ptr;
    using Resources = resource::Manager;

    virtual ~Extension_provider() noexcept = default;

    virtual uint32_t create_extension(json::Value const& extension_value, std::string const& name,
                                      Scene& scene, Resources& resources) noexcept = 0;
};

}  // namespace scene

#endif
