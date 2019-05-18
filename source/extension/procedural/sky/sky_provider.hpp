#ifndef SU_EXTENSION_PROCEDURAL_SKY_PROVIDER_HPP
#define SU_EXTENSION_PROCEDURAL_SKY_PROVIDER_HPP

#include "core/scene/extension_provider.hpp"

namespace scene {

class Loader;

namespace material {
class Provider;
}

}  // namespace scene

namespace procedural::sky {

void init(scene::Loader& loader, scene::material::Provider& material_provider) noexcept;

class Provider : public scene::Extension_provider {
  public:
    ~Provider() noexcept override final;

    void set_scene_loader(scene::Loader& loader) noexcept;
    void set_material_provider(scene::material::Provider& provider) noexcept;

    scene::prop::Prop_ref create_extension(json::Value const& extension_value,
                                           std::string const& name, scene::Scene& scene,
                                           resource::Manager& manager) noexcept override final;

  private:
    scene::Loader*             scene_loader_      = nullptr;
    scene::material::Provider* material_provider_ = nullptr;
};

}  // namespace procedural::sky

#endif
