#ifndef SU_CORE_SCENE_MATERIAL_PROVIDER_HPP
#define SU_CORE_SCENE_MATERIAL_PROVIDER_HPP

#include "base/json/json_types.hpp"
#include "resource/resource_provider.hpp"

namespace scene::material {

class Material;

class Provider final : public resource::Provider<Material> {
  public:
    enum class Tex { All, No, DWIM };

    Provider(bool no_tex, bool no_tex_dwim, bool force_debug_material);

    ~Provider() final;

    Material* load(std::string const& filename, Variants const& options, Resources& resources,
                   std::string& resolved_name) final;

    Material* load(void const* data, std::string const& source_name, Variants const& options,
                   Resources& resources) final;

    Material* create_fallback_material();

  private:
    Material* load(json::Value const& value, std::string_view mount_folder,
                   Resources& resources) const;

    Material* load_mix(json::Value const& value, Resources& resources) const;

    Material* load_debug(json::Value const& value, Resources& resources) const;

    Material* load_display(json::Value const& value, Resources& resources) const;

    Material* load_glass(json::Value const& value, Resources& resources) const;

    Material* load_light(json::Value const& value, Resources& resources) const;

    Material* load_metal(json::Value const& value, Resources& resources) const;

    Material* load_substitute(json::Value const& value, Resources& resources) const;

    Material* load_volumetric(json::Value const& value, Resources& resources) const;

    Tex tex_;

    bool force_debug_material_;

  public:
    static uint32_t max_sample_size();
};

}  // namespace scene::material

#endif
