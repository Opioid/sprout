#pragma once

#include "resource/resource_provider.hpp"
#include "material_sample_cache.hpp"
#include "cloth/cloth_sample.hpp"
#include "display/display_sample.hpp"
#include "glass/glass_sample.hpp"
#include "glass/glass_rough_sample.hpp"
#include "light/light_material_sample.hpp"
#include "metal/metal_sample.hpp"
#include "substitute/substitute_sample.hpp"
#include "substitute/substitute_sample_translucent.hpp"
#include "base/json/rapidjson_types.hpp"
#include <vector>

namespace image { namespace texture { class Texture_2D; }}

namespace scene { namespace material {

class Material;

typedef std::vector<std::shared_ptr<Material>> Materials;

struct Sampler_settings;

class Provider : public resource::Provider<Material> {
public:

	Provider(uint32_t num_threads);

	virtual std::shared_ptr<Material> load(const std::string& filename,
										   const memory::Variant_map& options,
										   resource::Manager& manager) final override;

	std::shared_ptr<Material> fallback_material() const;

private:

	std::shared_ptr<Material> load_cloth     (const rapidjson::Value& cloth_value,      resource::Manager& manager);
	std::shared_ptr<Material> load_display   (const rapidjson::Value& display_value,    resource::Manager& manager);
	std::shared_ptr<Material> load_glass     (const rapidjson::Value& glass_value,      resource::Manager& manager);
	std::shared_ptr<Material> load_light     (const rapidjson::Value& light_value,      resource::Manager& manager);
	std::shared_ptr<Material> load_metal     (const rapidjson::Value& metal_value,      resource::Manager& manager);
	std::shared_ptr<Material> load_substitute(const rapidjson::Value& substitute_value, resource::Manager& manager);

	struct Texture_description {
		std::string filename;
		std::string usage;
		int32_t     num_elements;
	};

	void read_sampler_settings(const rapidjson::Value& sampler_value, Sampler_settings& settings);
	void read_texture_description(const rapidjson::Value& texture_value, Texture_description& description);

	Generic_sample_cache<cloth::Sample>						cloth_cache_;
	Generic_sample_cache<display::Sample>					display_cache_;
	Generic_sample_cache<glass::Sample>						glass_cache_;
	Generic_sample_cache<glass::Sample_rough>				glass_rough_cache_;
	Generic_sample_cache<light::Sample>						light_cache_;
	Generic_sample_cache<metal::Sample_isotropic>			metal_iso_cache_;
	Generic_sample_cache<metal::Sample_anisotropic>			metal_aniso_cache_;
	Generic_sample_cache<substitute::Sample>				substitute_cache_;
	Generic_sample_cache<substitute::Sample_translucent>	substitute_transmissive_cache_;

	std::shared_ptr<material::Material> fallback_material_;
};

}}
