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
#include "prop/prop.hpp"
#include "resource/resource_manager.inl"
#include "scene.inl"
#include "shape/canopy.hpp"
#include "shape/celestial_disk.hpp"
#include "shape/cube.hpp"
#include "shape/disk.hpp"
#include "shape/infinite_sphere.hpp"
#include "shape/plane.hpp"
#include "shape/rectangle.hpp"
#include "shape/sphere.hpp"
#include "shape/triangle/triangle_mesh.hpp"
#include "shape/triangle/triangle_mesh_generator.hpp"
#include "take/take.hpp"

namespace scene {

Loader::Loader(resource::Manager& manager, Material* fallback_material) noexcept
    : resource_manager_(manager),
      canopy_(manager.store<Shape>(new shape::Canopy())),
      celestial_disk_(manager.store<Shape>(new shape::Celestial_disk())),
      cube_(manager.store<Shape>(new shape::Cube())),
      disk_(manager.store<Shape>(new shape::Disk())),
      infinite_sphere_(manager.store<Shape>(new shape::Infinite_sphere())),
      plane_(manager.store<Shape>(new shape::Plane())),
      rectangle_(manager.store<Shape>(new shape::Rectangle())),
      sphere_(manager.store<Shape>(new shape::Sphere())),
      null_shape_(manager.store<Shape>(new shape::Null())),
      fallback_material_(manager.store<Material>(fallback_material)) {}

Loader::~Loader() noexcept {}

bool Loader::load(std::string const& filename, std::string_view take_name, take::Take const& take,
                  Scene& scene) noexcept {
    logging::verbose("Loading scene %S...", filename);

    std::string_view const take_mount_folder = string::parent_directory(take_name);

    auto const camera = take.view.camera;

    scene.calculate_num_interpolation_frames(camera ? camera->frame_step() : 0,
                                             camera ? camera->frame_duration() : 0);

    uint32_t const parent_id = prop::Null;

    math::Transformation const parent_transformation{float3(0.f), float3(1.f),
                                                     quaternion::identity()};

    bool const success = load(filename, take_mount_folder, parent_id, parent_transformation, scene);

    if (success) {
        scene.finish();
    }

    resource_manager_.thread_pool().wait_async();

    return success;
}

void Loader::register_extension_provider(std::string const&  name,
                                         Extension_provider* provider) noexcept {
    extension_providers_[name] = provider;
}

void Loader::register_mesh_generator(std::string const&          name,
                                     shape::triangle::Generator* generator) noexcept {
    mesh_generators_[name] = generator;
}

Loader::Shape_ptr Loader::canopy() noexcept {
    return canopy_;
}

Loader::Shape_ptr Loader::celestial_disk() noexcept {
    return celestial_disk_;
}

Loader::Shape_ptr Loader::cube() noexcept {
    return cube_;
}

Loader::Shape_ptr Loader::null_shape() noexcept {
    return null_shape_;
}

void Loader::create_light(uint32_t prop_id, Scene& scene) noexcept {
    auto const shape = scene.prop_shape(prop_id);
    for (uint32_t i = 0, len = shape->num_parts(); i < len; ++i) {
        if (auto const material = scene.prop_material(prop_id, i); material->is_emissive()) {
            if (material->is_scattering_volume()) {
                if (shape->is_analytical() && material->has_emission_map()) {
                    scene.create_prop_volume_image_light(prop_id, i);
                } else {
                    scene.create_prop_volume_light(prop_id, i);
                }
            } else {
                if (shape->is_analytical() && material->has_emission_map()) {
                    scene.create_prop_image_light(prop_id, i);
                } else {
                    scene.create_prop_light(prop_id, i);
                }
            }
        }
    }
}

size_t Loader::num_bytes() const noexcept {
    return 0;
}

bool Loader::load(std::string const& filename, std::string_view take_mount_folder,
                  uint32_t parent_id, math::Transformation const& parent_transformation,
                  Scene& scene) noexcept {
    auto& filesystem = resource_manager_.filesystem();

    if (!take_mount_folder.empty()) {
        filesystem.push_mount(take_mount_folder);
    }

    std::string resolved_name;
    auto        stream_pointer = filesystem.read_stream(filename, resolved_name);

    if (!take_mount_folder.empty()) {
        filesystem.pop_mount();
    }

    if (!stream_pointer) {
        return false;
    }

    std::string const mount_folder(string::parent_directory(resolved_name));

    std::string error;
    auto const  root = json::parse(*stream_pointer, error);
    if (!root) {
        logging::push_error(error);
        return false;
    }

    Local_materials local_materials;

    if (auto const materials_node = root->FindMember("materials");
        root->MemberEnd() != materials_node) {
        read_materials(materials_node->value, resolved_name, local_materials);
    }

    filesystem.push_mount(mount_folder);

    for (auto const& n : root->GetObject()) {
        if ("entities" == n.name) {
            load_entities(n.value, parent_id, parent_transformation, local_materials, scene);
        }
    }

    filesystem.pop_mount();

    return true;
}

void Loader::read_materials(json::Value const& materials_value, std::string const& source_name,
                            Local_materials& local_materials) const noexcept {
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
                           Local_materials const& local_materials, Scene& scene) noexcept {
    if (!entities_value.IsArray()) {
        return;
    }

    for (auto const& e : entities_value.GetArray()) {
        if (auto const file_node = e.FindMember("file"); e.MemberEnd() != file_node) {
            std::string const filename = file_node->value.GetString();
            load(filename, "", parent_id, parent_transformation, scene);
            continue;
        }

        auto const type_node = e.FindMember("type");
        if (e.MemberEnd() == type_node) {
            continue;
        }

        std::string const type_name = type_node->value.GetString();

        std::string const name = json::read_string(e, "name");

        uint32_t entity_id = prop::Null;

        if ("Light" == type_name) {
            uint32_t const prop_id = load_prop(e, name, local_materials, scene);

            if (prop::Null != prop_id && scene.prop(prop_id)->visible_in_reflection()) {
                create_light(prop_id, scene);
            }

            entity_id = prop_id;
        } else if ("Prop" == type_name) {
            uint32_t const prop_id = load_prop(e, name, local_materials, scene);

            entity_id = prop_id;
        } else if ("Dummy" == type_name) {
            entity_id = scene.create_dummy();
        } else {
            entity_id = load_extension(type_name, e, name, scene);
        }

        if (prop::Null == entity_id) {
            continue;
        }

        math::Transformation transformation{float3(0.f), float3(1.f), quaternion::identity()};

        json::Value const* animation_value = nullptr;
        json::Value const* children        = nullptr;
        json::Value const* visibility      = nullptr;

        // Setting the following properties on the entity is deferred,
        // until after potential children are attached.
        for (auto& n : e.GetObject()) {
            if ("transformation" == n.name) {
                json::read_transformation(n.value, transformation);
            } else if ("animation" == n.name) {
                animation_value = &n.value;
            } else if ("entities" == n.name) {
                children = &n.value;
            } else if ("visibility" == n.name) {
                visibility = &n.value;
            }
        }

        animation::Animation* animation = nullptr;

        if (animation_value) {
            animation = animation::load(*animation_value, transformation, scene);
            if (animation) {
                scene.create_animation_stage(entity_id, animation);
            }
        }

        if (prop::Null != parent_id) {
            scene.prop_serialize_child(parent_id, entity_id);
        }

        if (!animation) {
            if (scene.prop_has_animated_frames(entity_id)) {
                scene.prop_set_transformation(entity_id, transformation);
            } else {
                if (prop::Null != parent_id) {
                    transformation = transformation.transform(parent_transformation);
                }

                scene.prop_set_world_transformation(entity_id, transformation);
            }
        }

        if (visibility) {
            set_visibility(entity_id, *visibility, scene);
        }

        if (children) {
            load_entities(*children, entity_id, transformation, local_materials, scene);
        }
    }
}

void Loader::set_visibility(uint32_t prop, json::Value const& visibility_value,
                            Scene& scene) noexcept {
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

uint32_t Loader::load_prop(json::Value const& prop_value, std::string const& name,
                           Local_materials const& local_materials, Scene& scene) noexcept {
    logging::verbose("Loading prop...");

    Shape_ptr shape = Shape_ptr::Null();

    json::Value const* visibility      = nullptr;
    json::Value const* materials_value = nullptr;

    for (auto& n : prop_value.GetObject()) {
        if ("shape" == n.name) {
            shape = load_shape(n.value);
        } else if ("materials" == n.name) {
            //   load_materials(n.value, mount_folder, local_materials, scene, materials);
            materials_value = &n.value;
        } else if ("visibility" == n.name) {
            visibility = &n.value;
        }
    }

    if (!shape.ptr) {
        return 0xFFFFFFFF;
    }

    uint32_t const num_materials = shape.ptr->num_materials();

    Materials materials;
    materials.reserve(num_materials);

    if (materials_value) {
        load_materials(*materials_value, local_materials, scene, materials);
    }

    if (1 == materials.size() && 1.f == materials[0].ptr->ior()) {
    } else {
        while (materials.size() < num_materials) {
            materials.push_back(fallback_material_);
        }
    }

    uint32_t const prop = scene.create_prop(shape, materials, name);

    // It is a annoying that this is done again in load_entities(),
    // but visibility information is already used when creating lights.
    // Should be improved at some point.
    if (visibility) {
        set_visibility(prop, *visibility, scene);
    }

    return prop;
}

uint32_t Loader::load_extension(std::string const& type, json::Value const& extension_value,
                                std::string const& name, Scene& scene) noexcept {
    if (auto p = extension_providers_.find(type); extension_providers_.end() != p) {
        return p->second->create_extension(extension_value, name, scene, resource_manager_);
    }

    return 0xFFFFFFFF;
}

Loader::Shape_ptr Loader::load_shape(json::Value const& shape_value) noexcept {
    if (std::string const type = json::read_string(shape_value, "type"); !type.empty()) {
        return shape(type, shape_value);
    }

    if (std::string const file = json::read_string(shape_value, "file"); !file.empty()) {
        return resource_manager_.load<shape::Shape>(file);
    }

    logging::error("Cannot create Shape: Neither shape nor type.");

    return Shape_ptr::Null();
}

Loader::Shape_ptr Loader::shape(std::string const& type, json::Value const& shape_value) const
    noexcept {
    if ("Canopy" == type) {
        return canopy_;
    } else if ("Celestial_disk" == type) {
        return celestial_disk_;
    } else if ("Cube" == type) {
        return cube_;
    } else if ("Disk" == type) {
        return disk_;
    } else if ("Infinite_sphere" == type) {
        return infinite_sphere_;
    } else if ("Plane" == type) {
        return plane_;
    } else if ("Rectangle" == type) {
        return rectangle_;
    } else if ("Sphere" == type) {
        return sphere_;
    } else {
        if (auto g = mesh_generators_.find(type); mesh_generators_.end() != g) {
            auto shape = g->second->create_mesh(shape_value, resource_manager_);
            if (!shape.ptr) {
                logging::error("Cannot create shape of type \"" + type + "\".");
            }

            return shape;

        } else {
            logging::error("Cannot create shape of type \"" + type + "\": Undefined type.");
        }
    }

    return Shape_ptr::Null();
}

void Loader::load_materials(json::Value const&     materials_value,
                            Local_materials const& local_materials, Scene& scene,
                            Materials& materials) const noexcept {
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

Loader::Material_ptr Loader::load_material(std::string const&     name,
                                           Local_materials const& local_materials,
                                           Scene&                 scene) const noexcept {
    // First, check if we maybe already have cached the material.
    if (auto material = resource_manager_.get<material::Material>(name); material.ptr) {
        return material;
    }

    // Otherwise, see if it is among the locally defined materials.
    if (auto const material_node = local_materials.materials.find(name);
        local_materials.materials.end() != material_node) {
        void const* data = reinterpret_cast<void const*>(material_node->second);

        if (auto material = resource_manager_.load<material::Material>(name, data,
                                                                       local_materials.source_name);
            material.ptr) {
            if (material.ptr->is_animated()) {
                scene.add_material(material.id);
            }

            return material;
        }
    }

    // Lastly, try loading the material from the filesystem.
    if (auto material = resource_manager_.load<material::Material>(name); material.ptr) {
        if (material.ptr->is_animated()) {
            scene.add_material(material.id);
        }

        return material;
    }

    logging::error("Using fallback for material %S: ", name);

    return fallback_material_;
}

}  // namespace scene
