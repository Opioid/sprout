#ifndef SU_CORE_SCENE_LOADER_HPP
#define SU_CORE_SCENE_LOADER_HPP

#include "material/material.hpp"
#include "base/json/json_types.hpp"
#include <istream>
#include <string>
#include <map>
#include <memory>

namespace file { class System; }

namespace resource { class Manager; }

namespace scene {

namespace entity {

class Entity;
class Extension_provider;

}

namespace surrounding { class Surrounding; }

namespace light { class Light; }

namespace volume { class Volume; }

namespace prop { class Prop; }

namespace shape {

class Shape;

namespace triangle {

class Generator;
class Provider;

}}

class Scene;

class Loader {

public:

	Loader(resource::Manager& manager, const Material_ptr& fallback_material);
    ~Loader();

	bool load(std::string const& filename, std::string const& take_name, Scene& scene);

	void register_extension_provider(std::string const& name, entity::Extension_provider* provider);
    void register_mesh_generator(std::string const& name, shape::triangle::Generator* generator);

	std::shared_ptr<shape::Shape> box();
	std::shared_ptr<shape::Shape> canopy();
	std::shared_ptr<shape::Shape> celestial_disk();

	size_t num_bytes() const;

private:

	void read_materials(json::Value const& materials_value);

	void load_entities(json::Value const& entities_value, entity::Entity* parent, Scene& scene);

	void set_visibility(entity::Entity* entity, json::Value const& visibility_value);

	prop::Prop* load_prop(json::Value const& prop_value, std::string const& name, Scene& scene);

	void load_light(json::Value const& light_value, prop::Prop* prop, Scene& scene);

	entity::Entity* load_extension(std::string const& type, json::Value const& extension_value,
								   std::string const& name, Scene& scene);

	std::shared_ptr<shape::Shape> load_shape(json::Value const& shape_value);

	std::shared_ptr<shape::Shape> shape(std::string const& type,
										json::Value const& shape_value) const;

	void load_materials(json::Value const& materials_value, Scene& scene,
						Materials& materials);

	Material_ptr load_material(std::string const& name, Scene& scene);

	resource::Manager& resource_manager_;

	std::shared_ptr<shape::Shape> box_;
	std::shared_ptr<shape::Shape> canopy_;
	std::shared_ptr<shape::Shape> celestial_disk_;
	std::shared_ptr<shape::Shape> disk_;
	std::shared_ptr<shape::Shape> infinite_sphere_;
	std::shared_ptr<shape::Shape> plane_;
	std::shared_ptr<shape::Shape> rectangle_;
	std::shared_ptr<shape::Shape> sphere_;

	Material_ptr fallback_material_;

	std::map<std::string, json::Value const*> local_materials_;
	std::string mount_folder_;

	std::map<std::string, entity::Extension_provider*> extension_providers_;
	std::map<std::string, shape::triangle::Generator*> mesh_generators_;
};

}

#endif
