#ifndef SU_EXTENSION_PROCEDURAL_SKY_PROVIDER_HPP
#define SU_EXTENSION_PROCEDURAL_SKY_PROVIDER_HPP

#include "core/scene/extension_provider.hpp"

namespace scene {
class Loader;
}  // namespace scene

namespace procedural::sky {

void init(scene::Loader& loader);

class Provider final : public scene::Extension_provider {
  public:
    ~Provider() final;

    void set_scene_loader(scene::Loader& loader);

    uint32_t create_extension(json::Value const& value, scene::Scene& scene,
                              Resources& resources) final;

  private:
    scene::Loader* scene_loader_ = nullptr;
};

}  // namespace procedural::sky

#endif
