#ifndef SU_CORE_SCENE_LOADER_HPP
#define SU_CORE_SCENE_LOADER_HPP

#include <map>
#include <string>
#include "base/json/json_types.hpp"
#include "material/material.hpp"

namespace file {
class System;
}

namespace resource {
class Manager;
}

namespace take {
struct Take;
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
    using Shape    = shape::Shape;
    using Material = material::Material;

    using Materials = std::vector<material::Material*>;

    Loader(resource::Manager& manager, Material& fallback_material);

    ~Loader();

    bool load(std::string const& filename, std::string_view take_name, take::Take const& take,
              Scene& scene);

    void register_extension_provider(std::string const& name, entity::Extension_provider* provider);
    void register_mesh_generator(std::string const& name, shape::triangle::Generator* generator);

    Shape* canopy();
    Shape* celestial_disk();
    Shape* cube();

    void create_light(prop::Prop* prop, Scene& scene);

    size_t num_bytes() const;

  private:
    void read_materials(json::Value const& materials_value);

    void load_entities(json::Value const& entities_value, entity::Entity* parent, Scene& scene);

    void set_visibility(entity::Entity* entity, json::Value const& visibility_value);

    prop::Prop* load_prop(json::Value const& prop_value, std::string const& name, Scene& scene);

    entity::Entity* load_extension(std::string const& type, json::Value const& extension_value,
                                   std::string const& name, Scene& scene);

    Shape* load_shape(json::Value const& shape_value);

    Shape* shape(std::string const& type, json::Value const& shape_value) const;

    void load_materials(json::Value const& materials_value, Scene& scene, Materials& materials);

    Material* load_material(std::string const& name, Scene& scene);

    resource::Manager& resource_manager_;

    Shape* canopy_;
    Shape* celestial_disk_;
    Shape* cube_;
    Shape* disk_;
    Shape* infinite_sphere_;
    Shape* plane_;
    Shape* rectangle_;
    Shape* sphere_;

    Material& fallback_material_;

    std::map<std::string, json::Value const*> local_materials_;

    std::string mount_folder_;

    std::map<std::string, entity::Extension_provider*> extension_providers_;
    std::map<std::string, shape::triangle::Generator*> mesh_generators_;
};

}  // namespace scene

#endif
