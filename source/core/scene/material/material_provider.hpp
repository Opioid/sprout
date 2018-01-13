#ifndef SU_CORE_SCENE_MATERIAL_PROVIDER_HPP
#define SU_CORE_SCENE_MATERIAL_PROVIDER_HPP

#include "resource/resource_provider.hpp"
#include "material_sample_cache.hpp"
#include "image/channels.hpp"
#include "image/texture/texture_types.hpp"
#include "base/json/json_types.hpp"
#include "base/math/vector3.hpp"
#include <vector>

namespace scene::material {

class Material;

using Material_ptr = std::shared_ptr<Material>;

struct Sampler_settings;

class Provider : public resource::Provider<Material> {

public:

	Provider();
	~Provider();

	virtual Material_ptr load(const std::string& filename, const memory::Variant_map& options,
							  resource::Manager& manager) override final;

	virtual Material_ptr load(const void* data, const std::string& mount_folder,
							  const memory::Variant_map& options,
							  resource::Manager& manager) override final;

	virtual size_t num_bytes() const override final;

	Material_ptr fallback_material() const;

private:

	Material_ptr load(const json::Value& value, const std::string& mount_folder,
					  resource::Manager& manager);

	Material_ptr load_cloth(const json::Value& cloth_value, resource::Manager& manager);

	Material_ptr load_debug(const json::Value& debug_value, resource::Manager& manager);

	Material_ptr load_display(const json::Value& display_value, resource::Manager& manager);

	Material_ptr load_glass(const json::Value& glass_value, resource::Manager& manager);

	Material_ptr load_light(const json::Value& light_value, resource::Manager& manager);

	Material_ptr load_matte(const json::Value& matte_value, resource::Manager& manager);

	Material_ptr load_metal(const json::Value& metal_value, resource::Manager& manager);

	Material_ptr load_metallic_paint(const json::Value& paint_value, resource::Manager& manager);

	Material_ptr load_mix(const json::Value& mix_value, resource::Manager& manager);

	Material_ptr load_sky(const json::Value& sky_value, resource::Manager& manager);

	Material_ptr load_substitute(const json::Value& substitute_value, resource::Manager& manager);

	Material_ptr load_volumetric(const json::Value& volumetric_value, resource::Manager& manager);

	struct Texture_description {
		std::string		filename;
		std::string		usage;
		image::Swizzle  swizzle;
		float2			scale;
		int32_t			num_elements;
	};

	static void read_sampler_settings(const json::Value& sampler_value,
									  Sampler_settings& settings);

	static void read_texture_description(const json::Value& texture_value,
										 Texture_description& description);

	static Texture_adapter create_texture(const Texture_description& description,
										  memory::Variant_map& options,
										  resource::Manager& manager);

	struct Coating_description {
		float3 color = float3(1.f);
		float  ior = 1.f;
		float  roughness = 0.f;
		float  thickness = 0.f;
		float  weight = 1.f;
		Texture_description normal_map_description;
		Texture_description weight_map_description;
	};

	static void read_coating_description(const json::Value& clearcoat_value,
										 Coating_description& description);

	static float3 read_hex_RGB(const std::string& text);

	static float3 read_color(const json::Value& color_value);

	static float3 read_spectrum(const json::Value& spectrum_value);

	Material_ptr fallback_material_;

public:

	static uint32_t max_sample_size();
};

}

#endif
