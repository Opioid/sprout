#pragma once

#include "shape/triangle/triangle_mesh_provider.hpp"
#include "material/material_provider.hpp"
#include "image/texture/texture_2d_provider.hpp"
#include "resource/resource_cache.hpp"
#include "base/json/rapidjson_types.hpp"
#include <istream>
#include <string>
#include <memory>

namespace file {

class System;

}

namespace scene {

namespace entity { class Entity; }

namespace surrounding { class Surrounding; }

namespace light { class Light; }

namespace shape { class Shape; }

class Scene;
class Prop;

class Loader {
public:

	Loader(file::System& file_system, uint32_t num_workers);
	~Loader();

	void load(std::istream& stream, Scene& scene);

private:

	void load_entities(const rapidjson::Value& entities_value, entity::Entity* parent, Scene& scene);

	Prop* load_prop(const rapidjson::Value& prop_value, Scene& scene);

	light::Light* load_light(const rapidjson::Value& light_value, Prop* prop, Scene& scene);

	std::shared_ptr<shape::Shape> load_shape(const rapidjson::Value& shape_value);

	std::shared_ptr<shape::Shape> shape(const std::string& type) const;

	void load_materials(const rapidjson::Value& materials_value, material::Materials& materials);

	std::shared_ptr<shape::Shape> canopy_;
	std::shared_ptr<shape::Shape> celestial_disk_;
	std::shared_ptr<shape::Shape> disk_;
	std::shared_ptr<shape::Shape> inverse_sphere_;
	std::shared_ptr<shape::Shape> plane_;
	std::shared_ptr<shape::Shape> sphere_;

	shape::triangle::Provider mesh_provider_;
	resource::Cache<shape::Shape> mesh_cache_;

	image::texture::Provider texture_provider_;
	resource::Cache<image::texture::Texture_2D> texture_cache_;

	material::Provider material_provider_;
	resource::Cache<material::IMaterial> material_cache_;
};

}
