#pragma once

#include "resource/resource_provider.hpp"
#include "material_sample_cache.hpp"
#include "base/json/json_types.hpp"
#include <vector>

namespace image { namespace texture { class Texture_2D; }}

namespace scene { namespace material {

namespace cloth { class Sample; }
namespace display { class Sample; }
namespace glass { class Sample; class Sample_rough; }
namespace light { class Sample; }
namespace metal { class Sample_isotropic; class Sample_anisotropic; }

namespace sky {

class Sample_clear;
class Material_clear;
class Sample_overcast;

}

namespace substitute {

class Sample;
class Sample_clearcoat;
class Sample_translucent;

}

class Material;

struct Sampler_settings;

class Provider : public resource::Provider<Material> {
public:

	Provider(uint32_t num_threads);
	~Provider();

	virtual std::shared_ptr<Material> load(const std::string& filename,
										   const memory::Variant_map& options,
										   resource::Manager& manager) final override;

	std::shared_ptr<Material> fallback_material() const;

	std::shared_ptr<sky::Material_clear> create_clear_sky();

private:

	std::shared_ptr<Material> load_cloth(const json::Value& cloth_value,
										 resource::Manager& manager);

	std::shared_ptr<Material> load_display(const json::Value& display_value,
										   resource::Manager& manager);

	std::shared_ptr<Material> load_glass(const json::Value& glass_value,
										 resource::Manager& manager);

	std::shared_ptr<Material> load_light(const json::Value& light_value,
										 resource::Manager& manager);

	std::shared_ptr<Material> load_metal(const json::Value& metal_value,
										 resource::Manager& manager);

	std::shared_ptr<Material> load_sky(const json::Value& sky_value,
									   resource::Manager& manager);

	std::shared_ptr<Material> load_substitute(const json::Value& substitute_value,
											  resource::Manager& manager);

	struct Texture_description {
		std::string filename;
		std::string usage;
		int32_t     num_elements;
	};

	static void read_sampler_settings(const json::Value& sampler_value,
									  Sampler_settings& settings);

	static void read_texture_description(const json::Value& texture_value,
										 Texture_description& description);

	struct Clearcoat_description {
		float ior = 1.f;
		float roughness = 0.f;
	};

	static void read_clearcoat_description(const json::Value& clearcoat_value,
										   Clearcoat_description& description);

	Generic_sample_cache<cloth::Sample>						cloth_cache_;
	Generic_sample_cache<display::Sample>					display_cache_;
	Generic_sample_cache<glass::Sample>						glass_cache_;
	Generic_sample_cache<glass::Sample_rough>				glass_rough_cache_;
	Generic_sample_cache<light::Sample>						light_cache_;
	Generic_sample_cache<metal::Sample_isotropic>			metal_iso_cache_;
	Generic_sample_cache<metal::Sample_anisotropic>			metal_aniso_cache_;
	Generic_sample_cache<sky::Sample_clear>					sky_clear_cache_;
	Generic_sample_cache<sky::Sample_overcast>				sky_overcast_cache_;
	Generic_sample_cache<substitute::Sample>				substitute_cache_;
	Generic_sample_cache<substitute::Sample_clearcoat>		substitute_clearcoat_cache_;
	Generic_sample_cache<substitute::Sample_translucent>	substitute_translucent_cache_;

	std::shared_ptr<material::Material> fallback_material_;
};

}}
