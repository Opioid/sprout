#ifndef SU_CORE_SCENE_LOADER_HPP
#define SU_CORE_SCENE_LOADER_HPP

#include <istream>
#include <map>
#include <memory>
#include <string>
#include "base/json/json_types.hpp"
#include "material/material.hpp"

namespace file {
class System;
}

namespace resource {
class Manager;
}

namespace scene {

namespace entity {

class Entity;
class Extension_provider;

}  // namespace entity

namespace surrounding {
class Surrounding;
}

namespace light {
class Light;
}

namespace volume {
class Volume;
}

namespace prop {
class Prop;
}

namespace shape {

class Shape;

namespace triangle {

class Generator;
class Provider;

}  // namespace triangle
}  // namespace shape

class Scene;

class Loader {
  public:
    using Shape_ptr = std::shared_ptr<shape::Shape>;

    Loader(resource::Manager& manager, Material_ptr const& fallback_material);
    ~Loader();

    bool load(std::string const& filename, std::string const& take_name, Scene& scene);

    void register_extension_provider(std::string const& name, entity::Extension_provider* provider);
    void register_mesh_generator(std::string const& name, shape::triangle::Generator* generator);

    Shape_ptr canopy();
    Shape_ptr celestial_disk();
    Shape_ptr cube();

    size_t num_bytes() const;

  private:
    void read_materials(json::Value const& materials_value);

    void load_entities(json::Value const& entities_value, entity::Entity* parent, Scene& scene);

    void set_visibility(entity::Entity* entity, json::Value const& visibility_value);

    prop::Prop* load_prop(json::Value const& prop_value, std::string const& name, Scene& scene);

    void load_light(json::Value const& light_value, prop::Prop* prop, Scene& scene);

    entity::Entity* load_extension(std::string const& type, json::Value const& extension_value,
                                   std::string const& name, Scene& scene);

    Shape_ptr load_shape(json::Value const& shape_value);

    Shape_ptr shape(std::string const& type, json::Value const& shape_value) const;

    void load_materials(json::Value const& materials_value, Scene& scene, Materials& materials);

    Material_ptr load_material(std::string const& name, Scene& scene);

    resource::Manager& resource_manager_;

    Shape_ptr canopy_;
    Shape_ptr celestial_disk_;
    Shape_ptr cube_;
    Shape_ptr disk_;
    Shape_ptr infinite_sphere_;
    Shape_ptr plane_;
    Shape_ptr rectangle_;
    Shape_ptr sphere_;

    Material_ptr fallback_material_;

    std::map<std::string, json::Value const*> local_materials_;

    std::string mount_folder_;

    std::map<std::string, entity::Extension_provider*> extension_providers_;
    std::map<std::string, shape::triangle::Generator*> mesh_generators_;
};

}  // namespace scene

#endif
