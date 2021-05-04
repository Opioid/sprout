#ifndef SU_CORE_SCENE_LOADER_HPP
#define SU_CORE_SCENE_LOADER_HPP

#include "base/json/json_types.hpp"
#include "base/memory/array.hpp"

#include <map>
#include <string>
#include <vector>

namespace math {
struct Transformation;
}

namespace resource {
class Manager;
}

namespace take {
struct Take;
}

namespace scene {

namespace camera {
class Camera;
}

namespace material {
class Material;
}

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
    using Camera    = camera::Camera;
    using Shape     = shape::Shape;
    using Material  = material::Material;
    using Materials = memory::Array<uint32_t>;
    using Resources = resource::Manager;

    Loader(Resources& resources, Material* fallback_material);

    ~Loader();

    bool load(std::string const& filename, take::Take const& take, Scene& scene);

    void register_extension_provider(std::string const& name, Extension_provider* provider);

    void register_mesh_generator(std::string const& name, shape::triangle::Generator* generator);

    uint32_t canopy() const;
    uint32_t distant_sphere() const;
    uint32_t cube() const;
    uint32_t null_shape() const;

    Materials& materials_buffer();

    uint32_t fallback_material() const;

  private:
    struct Local_materials {
        std::string source_name;

        std::map<std::string, json::Value const*> materials;
    };

    bool load(std::string const& filename, std::string_view take_mount_folder, uint32_t parent_id,
              math::Transformation const& parent_transformation, Scene& scene, Camera* camera,
              bool nested);

    void read_materials(json::Value const& materials_value, std::string const& source_name,
                        Local_materials& local_materials) const;

    void load_entities(json::Value const& entities_value, uint32_t parent_id,
                       math::Transformation const& parent_transformation,
                       Local_materials const& local_materials, Scene& scene, Camera* camera);

    static void set_visibility(uint32_t prop, json::Value const& visibility_value, Scene& scene);

    uint32_t load_prop(json::Value const& prop_value, Local_materials const& local_materials,
                       Scene& scene);

    uint32_t load_extension(std::string const& type, json::Value const& extension_value,
                            Scene& scene);

    uint32_t load_shape(json::Value const& shape_value);

    uint32_t shape(std::string const& type, json::Value const& shape_value) const;

    void load_materials(json::Value const& materials_value, Local_materials const& local_materials,
                        Scene& scene, Materials& materials) const;

    uint32_t load_material(std::string const& name, Local_materials const& local_materials,
                           Scene& scene) const;

    resource::Manager& resource_manager_;

    uint32_t canopy_;
    uint32_t cube_;
    uint32_t disk_;
    uint32_t distant_sphere_;
    uint32_t infinite_sphere_;
    uint32_t plane_;
    uint32_t rectangle_;
    uint32_t sphere_;
    uint32_t null_shape_;

    uint32_t fallback_material_;

    std::map<std::string, Extension_provider*> extension_providers_;

    std::map<std::string, shape::triangle::Generator*> mesh_generators_;

    Materials materials_;
};

}  // namespace scene

#endif
