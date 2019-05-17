#ifndef SU_SCENE_EXTENSION_HPP
#define SU_SCENE_EXTENSION_HPP

#include <cstdint>
#include "base/json/json_types.hpp"

namespace scene {

class Scene;

class Extension {
  public:
    virtual ~Extension() noexcept {}

    void init(uint32_t prop) noexcept {
        prop_ = prop;
    }

    virtual void set_parameters(json::Value const& parameters, Scene& scene) noexcept = 0;

    virtual void update(Scene& scene) noexcept = 0;

  protected:
    uint32_t prop_ = 0xFFFFFFFF;
};

}  // namespace scene

#endif
