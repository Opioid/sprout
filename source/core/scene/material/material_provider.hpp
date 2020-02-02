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
    Provider(bool force_debug_material);

    ~Provider() final;

    Material* load(std::string const& filename, Variants const& options, Resources& resources,
                   std::string& resolved_name) final;

    Material* load(void const* data, std::string const& source_name, Variants const& options,
                   Resources& resources) final;

    size_t num_bytes() const final;

    size_t num_bytes(Material const* resource) const final;

    Material* create_fallback_material();

  private:
    Material* load(json::Value const& value, std::string_view mount_folder, Resources& resources);

    Material* load_cloth(json::Value const& cloth_value, Resources& resources);

    Material* load_debug(json::Value const& debug_value, Resources& resources);

    Material* load_display(json::Value const& display_value, Resources& resources);

    Material* load_glass(json::Value const& glass_value, Resources& resources);

    Material* load_light(json::Value const& light_value, Resources& resources);

    Material* load_matte(json::Value const& matte_value, Resources& resources);

    Material* load_metal(json::Value const& metal_value, Resources& resources);

    Material* load_metallic_paint(json::Value const& paint_value, Resources& resources);

    Material* load_mix(json::Value const& mix_value, Resources& resources);

    Material* load_sky(json::Value const& sky_value, Resources& resources);

    Material* load_volumetric(json::Value const& volumetric_value, Resources& resources);

    bool force_debug_material_;

  public:
    static uint32_t max_sample_size();
};

}  // namespace scene::material

#endif
