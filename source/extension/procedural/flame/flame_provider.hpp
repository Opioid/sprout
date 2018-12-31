#ifndef SU_EXTENSION_PROCEDURAL_FLAME_PROVIDER_HPP
#define SU_EXTENSION_PROCEDURAL_FLAME_PROVIDER_HPP

#include "core/scene/entity/entity_extension_provider.hpp"

namespace scene {
class Loader;
}

namespace procedural::flame {

void init(scene::Loader& loader);

class Provider : public scene::entity::Extension_provider {
  public:
    ~Provider() noexcept override final;

    void set_scene_loader(scene::Loader& loader) noexcept;

    scene::entity::Entity* create_extension(json::Value const& extension_value, scene::Scene& scene,
                                            resource::Manager& manager) noexcept override final;

  private:
    scene::Loader* scene_loader_ = nullptr;
};

}  // namespace procedural::flame

#endif