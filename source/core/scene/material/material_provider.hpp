#pragma once

#include "resource/resource_provider.hpp"
#include "resource/resource_cache.hpp"
#include "glass/glass.hpp"
#include "substitute/substitute.hpp"
#include "base/json/rapidjson_types.hpp"

namespace image {

class Image;

}

namespace scene { namespace material {

class IMaterial;

class Provider : public resource::Provider<IMaterial> {
public:

	Provider(resource::Cache<image::Image>& image_cache, uint32_t num_workers);

	virtual std::shared_ptr<IMaterial> load(const std::string& filename, uint32_t flags = 0);

	std::shared_ptr<IMaterial> fallback_material() const;

private:

	std::shared_ptr<IMaterial> load_glass(const rapidjson::Value& glass_value);
	std::shared_ptr<IMaterial> load_substitute(const rapidjson::Value& substitute_value);

	resource::Cache<image::Image>& image_cache_;

	Sample_cache<glass::Sample> glass_cache_;
	Sample_cache<substitute::Sample> substitute_cache_;

	std::shared_ptr<material::IMaterial> fallback_material_;
};

}}
