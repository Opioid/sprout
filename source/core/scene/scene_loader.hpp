#pragma once

#include "material/material.hpp"
#include "base/json/rapidjson_types.hpp"
#include <istream>
#include <string>
#include <map>
#include <memory>

namespace file { class System; }

namespace resource { class Manager; }

namespace scene {

namespace entity { class Entity; }

namespace surrounding { class Surrounding; }

namespace light { class Light; }

namespace volume { class Volume; }

namespace shape {

class Shape;

namespace triangle {

class Generator;
class Provider;

}}

class Scene;
class Prop;

class Loader {
public:

	Loader(resource::Manager& manager, std::shared_ptr<material::Material> fallback_material);
    ~Loader();

	void load(std::istream& stream, Scene& scene);

    void register_mesh_generator(const std::string& name, shape::triangle::Generator* generator);

private:

	void load_entities(const rapidjson::Value& entities_value, entity::Entity* parent, Scene& scene);

	Prop* load_prop(const rapidjson::Value& prop_value, Scene& scene);

	void load_light(const rapidjson::Value& light_value, Prop* prop, Scene& scene);

	volume::Volume* load_volume(const rapidjson::Value& volume_value, Scene& scene);

	std::shared_ptr<shape::Shape> load_shape(const rapidjson::Value& shape_value);

    std::shared_ptr<shape::Shape> shape(const std::string& type, const rapidjson::Value& shape_value) const;

	void load_materials(const rapidjson::Value& materials_value, Scene& scene, material::Materials& materials);

	resource::Manager& resource_manager_;

	std::shared_ptr<shape::Shape> canopy_;
	std::shared_ptr<shape::Shape> celestial_disk_;
	std::shared_ptr<shape::Shape> disk_;
	std::shared_ptr<shape::Shape> infinite_sphere_;
	std::shared_ptr<shape::Shape> inverse_sphere_;
	std::shared_ptr<shape::Shape> plane_;
	std::shared_ptr<shape::Shape> sphere_;

	std::shared_ptr<material::Material> fallback_material_;

    std::map<std::string, shape::triangle::Generator*> mesh_generators_;
};

}
