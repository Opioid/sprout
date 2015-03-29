#pragma once

#include "prop/prop.hpp"
#include "shape/triangle/triangle_mesh_provider.hpp"
#include "material/material_provider.hpp"
#include "image/texture/texture_provider.hpp"
#include "resource/resource_cache.hpp"
#include "base/json/rapidjson_types.hpp"
#include <string>
#include <memory>

namespace scene {

namespace surrounding {

class Surrounding;

}

namespace light {

class Light;

}

namespace shape {

class Shape;

}

class Scene;

class Loader {
public:

	Loader(uint32_t num_workers);
	~Loader();

	bool load(const std::string& filename, Scene& scene);

private:

	surrounding::Surrounding* load_surrounding(const rapidjson::Value& surrounding_value) const;

	void load_entities(const rapidjson::Value& entities_value, Scene& scene);

	Prop* load_prop(const rapidjson::Value& prop_value, Scene& scene);

	light::Light* load_light(const rapidjson::Value& light_value, Scene& scene);

	std::shared_ptr<shape::Shape> load_shape(const rapidjson::Value& shape_value);

	std::shared_ptr<shape::Shape> shape(const std::string& type) const;

	void load_materials(const rapidjson::Value& materials_value, Prop::Materials& materials);

	std::shared_ptr<shape::Shape> plane_;
	std::shared_ptr<shape::Shape> sphere_;
	std::shared_ptr<shape::Shape> celestial_disk_;

	shape::triangle::Provider mesh_provider_;
	resource::Cache<shape::triangle::Mesh> mesh_cache_;

	image::texture::Provider texture_provider_;
	resource::Cache<image::texture::Texture> texture_cache_;

	material::Provider material_provider_;
	resource::Cache<material::IMaterial> material_cache_;
};

}
