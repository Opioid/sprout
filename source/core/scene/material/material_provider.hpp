#pragma once

#include "resource/resource_provider.hpp"
#include "resource/resource_cache.hpp"
#include "glass/glass.hpp"
#include "light/light_material.hpp"
#include "matte/matte.hpp"
#include "substitute/substitute_sample_cache.hpp"
#include "material_sample_cache.hpp"
#include "base/json/rapidjson_types.hpp"

namespace image { namespace texture {

class Texture_2D;

}}

namespace scene { namespace material {

class IMaterial;

class Provider : public resource::Provider<IMaterial> {
public:

	Provider(file::System& file_system,
			 resource::Cache<image::texture::Texture_2D>& texture_cache,
			 uint32_t num_workers);

	virtual std::shared_ptr<IMaterial> load(const std::string& filename, uint32_t flags = 0);

	std::shared_ptr<IMaterial> fallback_material() const;

private:

	std::shared_ptr<IMaterial> load_glass(const rapidjson::Value& glass_value);
	std::shared_ptr<IMaterial> load_light(const rapidjson::Value& light_value);
	std::shared_ptr<IMaterial> load_matte(const rapidjson::Value& matte_value);
	std::shared_ptr<IMaterial> load_substitute(const rapidjson::Value& substitute_value);

	std::shared_ptr<IMaterial> create(bool two_sided, bool thin,
									  std::shared_ptr<image::texture::Texture_2D> mask,
									  const math::float3& color,
									  float roughness,
									  float metallic);

	resource::Cache<image::texture::Texture_2D>& texture_cache_;

	Generic_sample_cache<glass::Sample> glass_cache_;
	Generic_sample_cache<light::Sample> light_cache_;
	Generic_sample_cache<matte::Sample> matte_cache_;
	substitute::Sample_cache substitute_cache_;

	std::shared_ptr<material::IMaterial> fallback_material_;
};

}}
