#ifndef SU_CORE_SCENE_EXTENSION_HPP
#define SU_CORE_SCENE_EXTENSION_HPP

#include "base/json/json_types.hpp"
#include "prop/prop.hpp"

#include <cstdint>

namespace scene {

class Scene;

class Extension {
  public:
    virtual ~Extension() = default;

    void init(uint32_t prop) {
        prop_ = prop;
    }

    virtual void set_parameters(json::Value const& parameters, Scene& scene) = 0;

    virtual void update(Scene& scene) = 0;

  protected:
    uint32_t prop_ = prop::Null;
};

}  // namespace scene

#endif
