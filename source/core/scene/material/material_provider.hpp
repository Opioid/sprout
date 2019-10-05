#ifndef SU_CORE_SCENE_MATERIAL_PROVIDER_HPP
#define SU_CORE_SCENE_MATERIAL_PROVIDER_HPP

#include "base/json/json_types.hpp"
#include "base/math/vector2.hpp"
#include "base/math/vector3.hpp"
#include "debug/debug_material.hpp"
#include "image/channels.hpp"
#include "image/texture/texture_types.hpp"
#include "material_sample_cache.hpp"
#include "resource/resource_provider.hpp"

namespace scene::material {

class Material;

struct Sampler_settings;

class Provider final : public resource::Provider<Material> {
  public:
    Provider(bool force_debug_material) noexcept;

    ~Provider() noexcept override final;

    Material* load(std::string const& filename, Variant_map const& options,
                   resource::Manager& manager, std::string& resolved_name) noexcept override final;

    Material* load(void const* data, std::string const& source_name, Variant_map const& options,
                   resource::Manager& manager) noexcept override final;

    size_t num_bytes() const noexcept override final;

    size_t num_bytes(Material const* resource) const noexcept override final;

	Material* create_fallback_material() noexcept;

  private:
    Material* load(json::Value const& value, std::string_view mount_folder,
                   resource::Manager& manager) noexcept;

    Material* load_cloth(json::Value const& cloth_value, resource::Manager& manager) noexcept;

    Material* load_debug(json::Value const& debug_value, resource::Manager& manager) noexcept;

    Material* load_display(json::Value const& display_value, resource::Manager& manager) noexcept;

    Material* load_glass(json::Value const& glass_value, resource::Manager& manager) noexcept;

    Material* load_light(json::Value const& light_value, resource::Manager& manager) noexcept;

    Material* load_matte(json::Value const& matte_value, resource::Manager& manager) noexcept;

    Material* load_metal(json::Value const& metal_value, resource::Manager& manager) noexcept;

    Material* load_metallic_paint(json::Value const& paint_value,
                                  resource::Manager& manager) noexcept;

    Material* load_mix(json::Value const& mix_value, resource::Manager& manager) noexcept;

    Material* load_sky(json::Value const& sky_value, resource::Manager& manager) noexcept;

    Material* load_volumetric(json::Value const& volumetric_value,
                              resource::Manager& manager) noexcept;

    bool force_debug_material_;

  public:
    static uint32_t max_sample_size() noexcept;
};

}  // namespace scene::material

#endif
