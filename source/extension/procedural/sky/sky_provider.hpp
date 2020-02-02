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

void init(scene::Loader& loader, scene::material::Provider& material_provider);

class Provider : public scene::Extension_provider {
  public:
    ~Provider() final;

    void set_scene_loader(scene::Loader& loader);
    void set_material_provider(scene::material::Provider& provider);

    uint32_t create_extension(json::Value const& extension_value, std::string const& name,
                              scene::Scene& scene, Resources& resources) final;

  private:
    scene::Loader*             scene_loader_      = nullptr;
    scene::material::Provider* material_provider_ = nullptr;
};

}  // namespace procedural::sky

#endif
