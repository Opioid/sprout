#ifndef SU_CORE_SCENE_MATERIAL_PROVIDER_HPP
#define SU_CORE_SCENE_MATERIAL_PROVIDER_HPP

#include <vector>
#include "base/json/json_types.hpp"
#include "base/math/vector2.hpp"
#include "base/math/vector3.hpp"
#include "image/channels.hpp"
#include "image/texture/texture_types.hpp"
#include "material_sample_cache.hpp"
#include "resource/resource_provider.hpp"

namespace scene::material {

class Material;

struct Sampler_settings;

class Provider final : public resource::Provider<Material> {
  public:
    Provider() noexcept;

    ~Provider() noexcept override final;

    Material* load(std::string const& filename, Variant_map const& options,
                   resource::Manager& manager) override final;

    Material* load(void const* data, std::string_view mount_folder, Variant_map const& options,
                   resource::Manager& manager) override final;

    size_t num_bytes() const noexcept override final;

    Material* fallback_material() const noexcept;

  private:
    Material* load(json::Value const& value, std::string_view mount_folder,
                   resource::Manager& manager);

    Material* load_cloth(json::Value const& cloth_value, resource::Manager& manager);

    Material* load_debug(json::Value const& debug_value, resource::Manager& manager);

    Material* load_display(json::Value const& display_value, resource::Manager& manager);

    Material* load_glass(json::Value const& glass_value, resource::Manager& manager);

    Material* load_light(json::Value const& light_value, resource::Manager& manager);

    Material* load_matte(json::Value const& matte_value, resource::Manager& manager);

    Material* load_metal(json::Value const& metal_value, resource::Manager& manager);

    Material* load_metallic_paint(json::Value const& paint_value, resource::Manager& manager);

    Material* load_mix(json::Value const& mix_value, resource::Manager& manager);

    Material* load_sky(json::Value const& sky_value, resource::Manager& manager);

    Material* load_substitute(json::Value const& substitute_value, resource::Manager& manager);

    Material* load_volumetric(json::Value const& volumetric_value, resource::Manager& manager);

    struct Texture_description {
        std::string filename;
        std::string usage;

        image::Swizzle swizzle;

        float2 scale;

        int32_t num_elements;
    };

    static void read_sampler_settings(json::Value const& sampler_value, Sampler_settings& settings);

    static void read_texture_description(json::Value const&   texture_value,
                                         Texture_description& description);

    static Texture_adapter create_texture(const Texture_description& description,
                                          Variant_map& options, resource::Manager& manager);

    struct Coating_description {
        float3 color = float3(1.f);

        float attenuation_distance = 1.f;
        float ior                  = 1.f;
        float roughness            = 0.f;
        float thickness            = 0.01f;

        bool in_nm = false;

        Texture_description normal_map_description;
        Texture_description thickness_map_description;
    };

    static void read_coating_description(json::Value const&   clearcoat_value,
                                         Coating_description& description);

    static float3 read_hex_RGB(std::string const& text);

    static float3 read_color(json::Value const& color_value);

    static float3 read_spectrum(json::Value const& spectrum_value);

    Material* fallback_material_;

  public:
    static uint32_t max_sample_size();
};

}  // namespace scene::material

#endif
