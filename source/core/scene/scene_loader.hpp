#ifndef SU_CORE_SCENE_LOADER_HPP
#define SU_CORE_SCENE_LOADER_HPP

#include "base/json/json_types.hpp"
#include "base/memory/array.hpp"
#include "material/material.hpp"
#include "resource/resource.hpp"

#include <map>
#include <string>
#include <vector>

namespace math {
struct Transformation;
}

namespace file {
class System;
}

namespace resource {
class Manager;

template <typename T>
struct Resource_ptr;
}  // namespace resource

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
struct Prop_ptr;
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
    using Shape        = shape::Shape;
    using Shape_ptr    = resource::Resource_ptr<Shape>;
    using Material     = material::Material;
    using Material_ptr = resource::Resource_ptr<Material>;
    using Materials    = memory::Array<Material_ptr>;

    Loader(resource::Manager& resources, Material* fallback_material) noexcept;

    ~Loader() noexcept;

    bool load(std::string const& filename, std::string_view take_name, take::Take const& take,
              Scene& scene) noexcept;

    void register_extension_provider(std::string const&  name,
                                     Extension_provider* provider) noexcept;

    void register_mesh_generator(std::string const&          name,
                                 shape::triangle::Generator* generator) noexcept;

    Shape_ptr canopy() noexcept;
    Shape_ptr celestial_disk() noexcept;
    Shape_ptr cube() noexcept;
    Shape_ptr null_shape() noexcept;

    void create_light(uint32_t prop_id, Scene& scene) noexcept;

	Materials& materials_buffer() noexcept;

	Material_ptr fallback_material() const noexcept;

    size_t num_bytes() const noexcept;

  private:
    struct Local_materials {
        std::string source_name;

        std::map<std::string, json::Value const*> materials;
    };

    bool load(std::string const& filename, std::string_view take_mount_folder, uint32_t parent_id,
              math::Transformation const& parent_transformation, Scene& scene) noexcept;

    void read_materials(json::Value const& materials_value, std::string const& source_name,
                        Local_materials& local_materials) const noexcept;

    void load_entities(json::Value const& entities_value, uint32_t parent_id,
                       math::Transformation const& parent_transformation,
                       Local_materials const& local_materials, Scene& scene) noexcept;

    static void set_visibility(uint32_t prop, json::Value const& visibility_value,
                               Scene& scene) noexcept;

    uint32_t load_prop(json::Value const& prop_value, std::string const& name,
                       Local_materials const& local_materials, Scene& scene) noexcept;

    uint32_t load_extension(std::string const& type, json::Value const& extension_value,
                            std::string const& name, Scene& scene) noexcept;

    Shape_ptr load_shape(json::Value const& shape_value) noexcept;

    Shape_ptr shape(std::string const& type, json::Value const& shape_value) const noexcept;

    void load_materials(json::Value const& materials_value, Local_materials const& local_materials,
                        Scene& scene, Materials& materials) const noexcept;

    Material_ptr load_material(std::string const& name, Local_materials const& local_materials,
                               Scene& scene) const noexcept;

    resource::Manager& resource_manager_;

    Shape_ptr canopy_;
    Shape_ptr celestial_disk_;
    Shape_ptr cube_;
    Shape_ptr disk_;
    Shape_ptr infinite_sphere_;
    Shape_ptr plane_;
    Shape_ptr rectangle_;
    Shape_ptr sphere_;
    Shape_ptr null_shape_;

    Material_ptr fallback_material_;

    std::map<std::string, Extension_provider*> extension_providers_;

    std::map<std::string, shape::triangle::Generator*> mesh_generators_;

	Materials materials_;
};

}  // namespace scene

#endif
