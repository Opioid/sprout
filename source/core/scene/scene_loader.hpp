#ifndef SU_CORE_SCENE_LOADER_HPP
#define SU_CORE_SCENE_LOADER_HPP

#include <map>
#include <string>
#include <vector>
#include "base/json/json_types.hpp"
#include "base/memory/array.hpp"
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
struct Prop_ref;
}  // namespace prop

namespace shape {

class Shape;

namespace triangle {

class Generator;
class Provider;

}  // namespace triangle
}  // namespace shape

class Extension_provider;

class Scene;

class Loader {
  public:
    using Shape     = shape::Shape;
    using Material  = material::Material;
    using Materials = memory::Array<material::Material*>;

    Loader(resource::Manager& manager, Material& fallback_material) noexcept;

    ~Loader() noexcept;

    bool load(std::string const& filename, std::string_view take_name, take::Take const& take,
              Scene& scene) noexcept;

    void register_extension_provider(std::string const&  name,
                                     Extension_provider* provider) noexcept;

    void register_mesh_generator(std::string const&          name,
                                 shape::triangle::Generator* generator) noexcept;

    Shape* canopy() noexcept;
    Shape* celestial_disk() noexcept;
    Shape* cube() noexcept;

    void create_light(uint32_t prop_id, Scene& scene) noexcept;

    size_t num_bytes() const noexcept;

  private:
    struct Local_materials {
        std::string source_name;

        std::map<std::string, json::Value const*> materials;
    };

    bool load(std::string const& filename, std::string_view take_mount_folder, uint32_t parent_id,
              Scene& scene) noexcept;

    void read_materials(json::Value const& materials_value, std::string const& source_name,
                        Local_materials& local_materials) const noexcept;

    void load_entities(json::Value const& entities_value, uint32_t parent_id,
                       Local_materials const& local_materials, Scene& scene) noexcept;

    static void set_visibility(uint32_t prop, json::Value const& visibility_value,
                               Scene& scene) noexcept;

    uint32_t load_prop(json::Value const& prop_value, std::string const& name,
                       Local_materials const& local_materials, Scene& scene) noexcept;

    uint32_t load_extension(std::string const& type, json::Value const& extension_value,
                            std::string const& name, Scene& scene) noexcept;

    Shape* load_shape(json::Value const& shape_value) noexcept;

    Shape* shape(std::string const& type, json::Value const& shape_value) const noexcept;

    void load_materials(json::Value const& materials_value, Local_materials const& local_materials,
                        Scene& scene, Materials& materials) const noexcept;

    Material* load_material(std::string const& name, Local_materials const& local_materials,
                            Scene& scene) const noexcept;

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

    std::map<std::string, Extension_provider*> extension_providers_;

    std::map<std::string, shape::triangle::Generator*> mesh_generators_;
};

}  // namespace scene

#endif
