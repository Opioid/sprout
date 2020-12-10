#ifndef SU_CORE_SCENE_MATERIAL_PROVIDER_HPP
#define SU_CORE_SCENE_MATERIAL_PROVIDER_HPP

#include "base/json/json_types.hpp"
#include "base/math/vector2.hpp"
#include "base/math/vector3.hpp"
#include "debug/debug_material.hpp"
#include "image/channels.hpp"
#include "image/texture/texture_types.hpp"
#include "resource/resource_provider.hpp"

namespace scene::material {

class Material;

struct Sampler_settings;

class Provider final : public resource::Provider<Material> {
  public:
    Provider(bool no_textures_dwim, bool force_debug_material);

    ~Provider() final;

    Material* load(std::string const& filename, Variants const& options, Resources& resources,
                   std::string& resolved_name) final;

    Material* load(void const* data, std::string const& source_name, Variants const& options,
                   Resources& resources) final;

    Material* create_fallback_material();

    struct Texture_description {
        std::string filename;

        std::string usage = "Color";

        image::Swizzle swizzle = image::Swizzle::XYZW;

        float scale = 1.f;

        int32_t num_elements = 1;
    };

    struct Coating_description {
        float3 color = float3(1.f);

        float attenuation_distance = 0.1f;
        float ior                  = 1.5f;
        float roughness            = 0.2f;
        float thickness            = 0.f;

        bool in_nm = false;

        Texture_description normal_map_description;
        Texture_description thickness_map_description;
    };

  private:
    Material* load(json::Value const& value, std::string_view mount_folder,
                   Resources& resources) const;

    Material* load_mix(json::Value const& mix_value, Resources& resources) const;

    Material* load_debug(json::Value const& debug_value, Resources& resources) const;

    Material* load_display(json::Value const& display_value, Resources& resources) const;

    Material* load_glass(json::Value const& glass_value, Resources& resources) const;

    Material* load_light(json::Value const& light_value, Resources& resources) const;

    Material* load_metal(json::Value const& metal_value, Resources& resources) const;

    Material* load_substitute(json::Value const& substitute_value, Resources& resources) const;

    Material* load_volumetric(json::Value const& volumetric_value, Resources& resources) const;

    Texture_description read_texture_description(json::Value const& texture_value) const;

    void read_coating_description(json::Value const& value, Coating_description& description) const;

    bool no_textures_dwim_;
    bool force_debug_material_;

  public:
    static uint32_t max_sample_size();
};

}  // namespace scene::material

#endif
