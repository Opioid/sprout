#include "scene_loader.hpp"
#include "animation/animation.hpp"
#include "animation/animation_loader.hpp"
#include "base/json/json.hpp"
#include "base/math/quaternion.inl"
#include "base/math/transformation.inl"
#include "base/math/vector3.inl"
#include "base/memory/array.inl"
#include "base/memory/variant_map.inl"
#include "base/string/string.hpp"
#include "base/thread/thread_pool.hpp"
#include "camera/camera.hpp"
#include "extension_provider.hpp"
#include "image/texture/texture_provider.hpp"
#include "light/light.hpp"
#include "logging/logging.hpp"
#include "material/material.inl"
#include "prop/prop.hpp"
#include "resource/resource_manager.inl"
#include "scene.inl"
#include "shape/canopy.hpp"
#include "shape/cube.hpp"
#include "shape/disk.hpp"
#include "shape/distant_sphere.hpp"
#include "shape/infinite_sphere.hpp"
#include "shape/null.hpp"
#include "shape/plane.hpp"
#include "shape/rectangle.hpp"
#include "shape/shape.inl"
#include "shape/sphere.hpp"
#include "shape/triangle/triangle_mesh.hpp"
#include "shape/triangle/triangle_mesh_generator.hpp"
#include "take/take.hpp"
#ifdef SU_DEBUG
#include "base/chrono/chrono.hpp"
#endif

namespace scene {

Loader::Loader(Resources& resources, Material* fallback_material)
    : resource_manager_(resources),
      canopy_(resources.store<Shape>(new shape::Canopy())),
      cube_(resources.store<Shape>(new shape::Cube())),
      disk_(resources.store<Shape>(new shape::Disk())),
      distant_sphere_(resources.store<Shape>(new shape::Distant_sphere())),
      infinite_sphere_(resources.store<Shape>(new shape::Infinite_sphere())),
      plane_(resources.store<Shape>(new shape::Plane())),
      rectangle_(resources.store<Shape>(new shape::Rectangle())),
      sphere_(resources.store<Shape>(new shape::Sphere())),
      null_shape_(resources.store<Shape>(new shape::Null())),
      fallback_material_(resources.store<Material>(fallback_material)) {}

Loader::~Loader() = default;

bool Loader::load(std::string const& filename, take::Take const& take, Scene& scene) {
    LOGGING_VERBOSE("Loading scene %S...", filename);

    std::string_view const take_mount_folder = string::parent_directory(take.resolved_name);

    auto const camera = take.view.camera;

    scene.calculate_num_interpolation_frames(camera ? camera->frame_step() : 0,
                                             camera ? camera->frame_duration() : 0);

    uint32_t const parent_id = prop::Null;

    math::Transformation const parent_transformation{float3(0.f), float3(1.f),
                                                     quaternion::Identity};

    bool const success = load(filename, take_mount_folder, parent_id, parent_transformation, scene,
                              camera, false);

    resource_manager_.threads().wait_async();

    return success;
}

void Loader::register_extension_provider(std::string const& name, Extension_provider* provider) {
    extension_providers_[name] = provider;
}

void Loader::register_mesh_generator(std::string const&          name,
                                     shape::triangle::Generator* generator) {
    mesh_generators_[name] = generator;
}

uint32_t Loader::canopy() const {
    return canopy_;
}

uint32_t Loader::distant_sphere() const {
    return distant_sphere_;
}

uint32_t Loader::cube() const {
    return cube_;
}

uint32_t Loader::null_shape() const {
    return null_shape_;
}

Loader::Materials& Loader::materials_buffer() {
    return materials_;
}

uint32_t Loader::fallback_material() const {
    return fallback_material_;
}

bool Loader::load(std::string const& filename, std::string_view take_mount_folder,
                  uint32_t parent_id, math::Transformation const& parent_transformation,
                  Scene& scene, Camera* camera, bool nested) {
    auto& filesystem = resource_manager_.filesystem();

    if (!take_mount_folder.empty()) {
        filesystem.push_mount(take_mount_folder);
    }

#ifdef SU_DEBUG
    auto const loading_start = std::chrono::high_resolution_clock::now();
#endif

    std::string resolved_name;
    auto        stream = filesystem.read_stream(filename, resolved_name);

    if (!take_mount_folder.empty()) {
        filesystem.pop_mount();
    }

    if (!stream) {
        if (nested) {
            logging::error("Loading scene %S: ", filename);
        }

        return false;
    }

    std::string error;
    auto const  root = json::parse(*stream, error);

    stream.close();

    if (root.HasParseError()) {
        if (nested) {
            logging::error(error);
        } else {
            logging::push_error(error);
        }

        return false;
    }

    LOGGING_VERBOSE("Parsing scene %f s", chrono::seconds_since(loading_start));

    Local_materials local_materials;

    if (auto const materials_node = root.FindMember("materials");
        root.MemberEnd() != materials_node) {
        read_materials(materials_node->value, resolved_name, local_materials);
    }

    std::string const mount_folder(string::parent_directory(resolved_name));

    filesystem.push_mount(mount_folder);

    for (auto const& n : root.GetObject()) {
        if ("entities" == n.name) {
            load_entities(n.value, parent_id, parent_transformation, local_materials, scene,
                          camera);
        }
    }

    filesystem.pop_mount();

    return true;
}

void Loader::read_materials(json::Value const& materials_value, std::string const& source_name,
                            Local_materials& local_materials) const {
    if (!materials_value.IsArray()) {
        return;
    }

    local_materials.source_name = source_name;

    for (auto const& m : materials_value.GetArray()) {
        auto const name_node = m.FindMember("name");
        if (m.MemberEnd() == name_node) {
            continue;
        }

        std::string const name = name_node->value.GetString();

        local_materials.materials[name] = &m;
    }
}

void Loader::load_entities(json::Value const& entities_value, uint32_t parent_id,
                           math::Transformation const& parent_transformation,
                           Local_materials const& local_materials, Scene& scene, Camera* camera) {
    if (!entities_value.IsArray()) {
        return;
    }

    for (auto const& e : entities_value.GetArray()) {
        if (auto const file_node = e.FindMember("file"); e.MemberEnd() != file_node) {
            std::string const filename = file_node->value.GetString();
            load(filename, "", parent_id, parent_transformation, scene, camera, true);
            continue;
        }

        auto const type_node = e.FindMember("type");
        if (e.MemberEnd() == type_node) {
            continue;
        }

        std::string const type_name = type_node->value.GetString();

        uint32_t entity_id = prop::Null;

        if ("Light" == type_name) {
            uint32_t const prop_id = load_prop(e, local_materials, scene);

            if (prop::Null != prop_id && scene.prop(prop_id)->visible_in_reflection()) {
                scene.create_light(prop_id);
            }

            entity_id = prop_id;
        } else if ("Prop" == type_name) {
            uint32_t const prop_id = load_prop(e, local_materials, scene);

            entity_id = prop_id;
        } else if ("Dummy" == type_name) {
            entity_id = scene.create_entity();
        } else if ("Camera" == type_name && camera) {
            entity_id = camera->entity();

            if (prop::Null == entity_id) {
                entity_id = scene.create_entity();
                camera->set_entity(entity_id);
            }
        } else {
            entity_id = load_extension(type_name, e, scene);
        }

        if (prop::Null == entity_id) {
            continue;
        }

        math::Transformation trafo{float3(0.f), float3(1.f), quaternion::Identity};

        json::Value const* animation_value = nullptr;
        json::Value const* children        = nullptr;
        json::Value const* visibility      = nullptr;

        // Setting the following properties on the entity is deferred,
        // until after potential children are attached.
        for (auto& n : e.GetObject()) {
            if ("transformation" == n.name) {
                json::read_transformation(n.value, trafo);
            } else if ("animation" == n.name) {
                animation_value = &n.value;
            } else if ("entities" == n.name) {
                children = &n.value;
            } else if ("visibility" == n.name) {
                visibility = &n.value;
            }
        }

        if (trafo.scale[1] <= 0.f && trafo.scale[2] <= 0.f &&
            1 == scene.prop_shape(entity_id)->num_parts()) {
            auto const material = scene.prop_material(entity_id, 0);

            if (material->is_heterogeneous_volume()) {
                auto const desc = material->useful_texture_description(scene);

                float const voxel_scale = trafo.scale[0];

                trafo.scale = 0.5f * voxel_scale * float3(desc.dimensions());

                trafo.position += trafo.scale + voxel_scale * float3(desc.offset());
            }
        }

        bool const animation = animation_value
                                   ? animation::load(*animation_value, trafo, entity_id, scene)
                                   : false;

        if (prop::Null != parent_id) {
            scene.prop_serialize_child(parent_id, entity_id);
        }

        if (!animation) {
            if (scene.prop_has_animated_frames(entity_id)) {
                scene.prop_set_transformation(entity_id, trafo);
            } else {
                if (prop::Null != parent_id) {
                    trafo = transform(trafo, parent_transformation);
                }

                scene.prop_set_world_transformation(entity_id, trafo);
            }
        }

        if (visibility) {
            set_visibility(entity_id, *visibility, scene);
        }

        if (children) {
            load_entities(*children, entity_id, trafo, local_materials, scene, camera);
        }
    }
}

void Loader::set_visibility(uint32_t prop, json::Value const& visibility_value, Scene& scene) {
    bool in_camera     = true;
    bool in_reflection = true;
    bool in_shadow     = true;

    for (auto& n : visibility_value.GetObject()) {
        if ("in_camera" == n.name) {
            in_camera = json::read_bool(n.value);
        } else if ("in_reflection" == n.name) {
            in_reflection = json::read_bool(n.value);
        } else if ("in_shadow" == n.name) {
            in_shadow = json::read_bool(n.value);
        }
    }

    scene.prop_set_visibility(prop, in_camera, in_reflection, in_shadow);
}

uint32_t Loader::load_prop(json::Value const& prop_value, Local_materials const& local_materials,
                           Scene& scene) {
    uint32_t shape = resource::Null;

    json::Value const* visibility      = nullptr;
    json::Value const* materials_value = nullptr;

    for (auto& n : prop_value.GetObject()) {
        if ("shape" == n.name) {
            shape = load_shape(n.value);
        } else if ("materials" == n.name) {
            materials_value = &n.value;
        } else if ("visibility" == n.name) {
            visibility = &n.value;
        }
    }

    if (resource::Null == shape) {
        return prop::Null;
    }

    uint32_t const num_materials = scene.shape(shape)->num_materials();

    materials_.reserve(num_materials);

    if (materials_value) {
        load_materials(*materials_value, local_materials, scene, materials_);
    }

    if (1 == materials_.size() && 1.f == scene.material(materials_[0])->ior()) {
    } else {
        while (materials_.size() < num_materials) {
            materials_.push_back(fallback_material_);
        }
    }

    uint32_t const prop = scene.create_prop(shape, materials_.data());

    // It is a annoying that this is done again in load_entities(),
    // but visibility information is already used when creating lights.
    // Should be improved at some point.
    if (visibility) {
        set_visibility(prop, *visibility, scene);
    }

    return prop;
}

uint32_t Loader::load_extension(std::string const& type, json::Value const& extension_value,
                                Scene& scene) {
    if (auto p = extension_providers_.find(type); extension_providers_.end() != p) {
        return p->second->create_extension(extension_value, scene, resource_manager_);
    }

    return prop::Null;
}

uint32_t Loader::load_shape(json::Value const& shape_value) {
    if (std::string const type = json::read_string(shape_value, "type"); !type.empty()) {
        return shape(type, shape_value);
    }

    if (std::string const file = json::read_string(shape_value, "file"); !file.empty()) {
        return resource_manager_.load<shape::Shape>(file).id;
    }

    logging::error("Cannot create Shape: Neither shape nor type.");

    return resource::Null;
}

uint32_t Loader::shape(std::string const& type, json::Value const& shape_value) const {
    if ("Canopy" == type) {
        return canopy_;
    }
    if ("Cube" == type) {
        return cube_;
    }
    if ("Disk" == type) {
        return disk_;
    }
    if ("Distant_sphere" == type) {
        return distant_sphere_;
    }
    if ("Infinite_sphere" == type) {
        return infinite_sphere_;
    }
    if ("Plane" == type) {
        return plane_;
    }
    if ("Rectangle" == type) {
        return rectangle_;
    }
    if ("Sphere" == type) {
        return sphere_;
    }

    if (auto g = mesh_generators_.find(type); mesh_generators_.end() != g) {
        auto shape = g->second->create_mesh(shape_value, resource_manager_);
        if (resource::Null == shape.id) {
            logging::error("Cannot create shape of type \"" + type + "\".");
        }

        return shape.id;
    }

    logging::error("Cannot create shape of type \"" + type + "\": Undefined type.");

    return resource::Null;
}

void Loader::load_materials(json::Value const&     materials_value,
                            Local_materials const& local_materials, Scene& scene,
                            Materials& materials) const {
    if (!materials_value.IsArray()) {
        return;
    }

    for (auto const& m : materials_value.GetArray()) {
        materials.push_back(load_material(m.GetString(), local_materials, scene));

        if (materials.full()) {
            break;
        }
    }
}

uint32_t Loader::load_material(std::string const& name, Local_materials const& local_materials,
                               Scene& scene) const {
    // First, check if we maybe already have cached the material.
    if (auto material = resource_manager_.get<Material>(name); material.ptr) {
        return material.id;
    }

    // Otherwise, see if it is among the locally defined materials.
    if (auto const material_node = local_materials.materials.find(name);
        local_materials.materials.end() != material_node) {
        void const* data = reinterpret_cast<void const*>(material_node->second);

        if (auto material = resource_manager_.load<Material>(name, data,
                                                             local_materials.source_name);
            material.ptr) {
            material.ptr->commit(resource_manager_.threads(), scene);
            return material.id;
        }
    }

    // Lastly, try loading the material from the filesystem.
    if (auto material = resource_manager_.load<Material>(name); material.ptr) {
        material.ptr->commit(resource_manager_.threads(), scene);
        return material.id;
    }

    logging::error("Using fallback for material %S: ", name);

    return fallback_material_;
}

}  // namespace scene
