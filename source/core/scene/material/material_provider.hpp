#pragma once

#include "resource/resource_provider.hpp"
#include "resource/resource_cache.hpp"
#include "material_sample_cache.hpp"
#include "base/json/rapidjson_types.hpp"
#include <vector>

namespace image { namespace texture { class Texture_2D; }}

namespace scene { namespace material {

namespace cloth { class Sample; }
namespace display { class Sample; }
namespace glass { class Sample; class Sample_rough; }
namespace light { class Sample; }
namespace metal { class Sample_isotropic; class Sample_anisotropic; }
namespace substitute { class Sample; }

class Material;

typedef std::vector<std::shared_ptr<Material>> Materials;

struct Sampler_settings;

class Provider : public resource::Provider<Material> {
public:

	Provider(file::System& file_system, thread::Pool& thread_pool,
			 resource::Cache<image::texture::Texture_2D>& texture_cache);

	virtual std::shared_ptr<Material> load(const std::string& filename, const memory::Variant_map& options);

	std::shared_ptr<Material> fallback_material() const;

private:

	std::shared_ptr<Material> load_cloth(const rapidjson::Value& cloth_value);
	std::shared_ptr<Material> load_display(const rapidjson::Value& display_value);
	std::shared_ptr<Material> load_glass(const rapidjson::Value& glass_value);
	std::shared_ptr<Material> load_light(const rapidjson::Value& light_value);
	std::shared_ptr<Material> load_metal(const rapidjson::Value& metal_value);
	std::shared_ptr<Material> load_substitute(const rapidjson::Value& substitute_value);

	struct Texture_description {
		std::string filename;
		std::string usage;
		int32_t     num_elements;
	};

	void read_sampler_settings(const rapidjson::Value& sampler_value, Sampler_settings& settings);
	void read_texture_description(const rapidjson::Value& texture_value, Texture_description& description);

	resource::Cache<image::texture::Texture_2D>& texture_cache_;

	Generic_sample_cache<cloth::Sample>				cloth_cache_;
	Generic_sample_cache<display::Sample>			display_cache_;
	Generic_sample_cache<glass::Sample>				glass_cache_;
	Generic_sample_cache<glass::Sample_rough>		glass_rough_cache_;
	Generic_sample_cache<light::Sample>				light_cache_;
	Generic_sample_cache<metal::Sample_isotropic>   metal_iso_cache_;
	Generic_sample_cache<metal::Sample_anisotropic> metal_aniso_cache_;
	Generic_sample_cache<substitute::Sample>		substitute_cache_;

	std::shared_ptr<material::Material> fallback_material_;
};

}}
