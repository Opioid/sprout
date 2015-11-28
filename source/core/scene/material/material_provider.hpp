#pragma once

#include "resource/resource_provider.hpp"
#include "resource/resource_cache.hpp"
#include "material_sample_cache.hpp"
#include "base/json/rapidjson_types.hpp"
#include <vector>

namespace image { namespace texture {

class Texture_2D;

}}

namespace scene { namespace material {

namespace cloth { class Sample; }
namespace glass { class Sample; }
namespace light { class Sample; }
namespace metal { class Sample_iso; class Sample_aniso; }
namespace substitute { class Sample; }

class IMaterial;

typedef std::vector<std::shared_ptr<IMaterial>> Materials;

class Provider : public resource::Provider<IMaterial> {
public:

	Provider(file::System& file_system,
			 resource::Cache<image::texture::Texture_2D>& texture_cache,
			 uint32_t num_workers);

	virtual std::shared_ptr<IMaterial> load(const std::string& filename, uint32_t flags = 0);

	std::shared_ptr<IMaterial> fallback_material() const;

private:

	std::shared_ptr<IMaterial> load_cloth(const rapidjson::Value& cloth_value);
	std::shared_ptr<IMaterial> load_glass(const rapidjson::Value& glass_value);
	std::shared_ptr<IMaterial> load_light(const rapidjson::Value& light_value);
	std::shared_ptr<IMaterial> load_metal(const rapidjson::Value& metal_value);
	std::shared_ptr<IMaterial> load_substitute(const rapidjson::Value& substitute_value);

	resource::Cache<image::texture::Texture_2D>& texture_cache_;

	Generic_sample_cache<cloth::Sample>		  cloth_cache_;
	Generic_sample_cache<glass::Sample>		  glass_cache_;
	Generic_sample_cache<light::Sample>		  light_cache_;
	Generic_sample_cache<metal::Sample_iso>   metal_iso_cache_;
	Generic_sample_cache<metal::Sample_aniso> metal_aniso_cache_;
	Generic_sample_cache<substitute::Sample>  substitute_cache_;

	std::shared_ptr<material::IMaterial> fallback_material_;
};

}}
