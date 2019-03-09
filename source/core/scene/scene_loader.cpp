#include "scene_loader.hpp"
#include "animation/animation.hpp"
#include "animation/animation_loader.hpp"
#include "base/json/json.hpp"
#include "base/math/quaternion.inl"
#include "base/math/vector3.inl"
#include "base/memory/variant_map.inl"
#include "base/string/string.hpp"
#include "base/thread/thread_pool.hpp"
#include "camera/camera.hpp"
#include "entity/dummy.hpp"
#include "entity/entity_extension_provider.hpp"
#include "image/texture/texture_provider.hpp"
#include "light/light.hpp"
#include "logging/logging.hpp"
#include "prop/prop.hpp"
#include "resource/resource_manager.inl"
#include "scene.hpp"
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

Loader::Loader(resource::Manager& manager, Material& fallback_material) noexcept
    : resource_manager_(manager),
      canopy_(new shape::Canopy()),
      celestial_disk_(new shape::Celestial_disk()),
      cube_(new shape::Cube()),
      disk_(new shape::Disk()),
      infinite_sphere_(new shape::Infinite_sphere()),
      plane_(new shape::Plane()),
      rectangle_(new shape::Rectangle()),
      sphere_(new shape::Sphere()),
      fallback_material_(fallback_material) {}

Loader::~Loader() noexcept {
    delete sphere_;
    delete rectangle_;
    delete plane_;
    delete infinite_sphere_;
    delete disk_;
    delete cube_;
    delete celestial_disk_;
    delete canopy_;
}

bool Loader::load(std::string const& filename, std::string_view take_name, take::Take const& take,
                  Scene& scene) noexcept {
    std::string_view const take_mount_folder = string::parent_directory(take_name);

    bool const success = load(filename, take_mount_folder, nullptr, scene);

    if (success) {
        scene.finish(take.view.camera ? take.view.camera->frame_step() : 0,
                     take.view.camera ? take.view.camera->frame_duration() : 0);
    }

    resource_manager_.thread_pool().wait_async();

    return success;
}

void Loader::register_extension_provider(std::string const&          name,
                                         entity::Extension_provider* provider) noexcept {
    extension_providers_[name] = provider;
}

void Loader::register_mesh_generator(std::string const&          name,
                                     shape::triangle::Generator* generator) noexcept {
    mesh_generators_[name] = generator;
}

Scene::Shape* Loader::canopy() noexcept {
    return canopy_;
}

Scene::Shape* Loader::celestial_disk() noexcept {
    return celestial_disk_;
}

Scene::Shape* Loader::cube() noexcept {
    return cube_;
}

void Loader::create_light(prop::Prop* prop, Scene& scene) noexcept {
    for (uint32_t i = 0, len = prop->shape()->num_parts(); i < len; ++i) {
        if (auto const material = prop->material(i); material->is_emissive()) {
            if (material->is_scattering_volume()) {
                if (prop->shape()->is_analytical() && material->has_emission_map()) {
                    scene.create_prop_volume_image_light(prop, i);
                } else {
                    scene.create_prop_volume_light(prop, i);
                }
            } else {
                if (prop->shape()->is_analytical() && material->has_emission_map()) {
                    scene.create_prop_image_light(prop, i);
                } else {
                    scene.create_prop_light(prop, i);
                }
            }
        }
    }
}

size_t Loader::num_bytes() const noexcept {
    return 0;
}

bool Loader::load(std::string const& filename, std::string_view take_mount_folder,
                  entity::Entity* parent, Scene& scene) noexcept {
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
        logging::error("Scene \"" + filename + "\":" + error);
        return false;
    }

    Local_materials local_materials;

    if (auto const materials_node = root->FindMember("materials");
        root->MemberEnd() != materials_node) {
        read_materials(materials_node->value, local_materials);
    }

    filesystem.push_mount(mount_folder);

    for (auto const& n : root->GetObject()) {
        if ("entities" == n.name) {
            load_entities(n.value, parent, mount_folder, local_materials, scene);
        }
    }

    filesystem.pop_mount();

    return true;
}

void Loader::read_materials(json::Value const& materials_value,
                            Local_materials&   local_materials) const noexcept {
    if (!materials_value.IsArray()) {
        return;
    }

    for (auto const& m : materials_value.GetArray()) {
        auto const name_node = m.FindMember("name");
        if (m.MemberEnd() == name_node) {
            continue;
        }

        std::string const name = name_node->value.GetString();

        local_materials[name] = &m;
    }
}

void Loader::load_entities(json::Value const& entities_value, entity::Entity* parent,
                           std::string_view mount_folder, Local_materials const& local_materials,
                           Scene& scene) noexcept {
    if (!entities_value.IsArray()) {
        return;
    }

    for (auto const& e : entities_value.GetArray()) {
        auto const file_node = e.FindMember("file");
        if (e.MemberEnd() != file_node) {
            std::string const filename = file_node->value.GetString();
            load(filename, "", parent, scene);
            continue;
        }

        auto const type_node = e.FindMember("type");
        if (e.MemberEnd() == type_node) {
            continue;
        }

        std::string const type_name = type_node->value.GetString();

        std::string const name = json::read_string(e, "name");

        entity::Entity* entity = nullptr;

        if ("Light" == type_name) {
            prop::Prop* prop = load_prop(e, name, mount_folder, local_materials, scene);

            if (prop && prop->visible_in_reflection()) {
                create_light(prop, scene);
            }

            entity = prop;
        } else if ("Prop" == type_name) {
            entity = load_prop(e, name, mount_folder, local_materials, scene);
        } else if ("Dummy" == type_name) {
            entity = scene.create_dummy();
        } else {
            entity = load_extension(type_name, e, name, scene);
        }

        if (!entity) {
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

        if (children) {
            load_entities(*children, entity, mount_folder, local_materials, scene);
        }

        if (parent) {
            parent->attach(entity);
        }

        animation::Animation* animation = nullptr;

        if (animation_value) {
            animation = animation::load(*animation_value, transformation, scene);
            if (animation) {
                scene.create_animation_stage(entity, animation);
            }
        }

        if (!animation) {
            entity->allocate_local_frame();
            entity->set_transformation(transformation);
        }

        if (visibility) {
            set_visibility(entity, *visibility);
        }
    }
}

void Loader::set_visibility(entity::Entity* entity, json::Value const& visibility_value) noexcept {
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

    entity->set_visibility(in_camera, in_reflection, in_shadow);
}

prop::Prop* Loader::load_prop(json::Value const& prop_value, std::string const& name,
                              std::string_view mount_folder, Local_materials const& local_materials,
                              Scene& scene) noexcept {
    Shape*    shape = nullptr;
    Materials materials;

    json::Value const* visibility = nullptr;

    for (auto& n : prop_value.GetObject()) {
        if ("shape" == n.name) {
            shape = load_shape(n.value);
        } else if ("materials" == n.name) {
            load_materials(n.value, mount_folder, local_materials, scene, materials);
        } else if ("visibility" == n.name) {
            visibility = &n.value;
        }
    }

    if (!shape) {
        return nullptr;
    }

    if (1 == materials.size() && 1.f == materials[0]->ior()) {
    } else {
        while (materials.size() < shape->num_parts()) {
            materials.push_back(&fallback_material_);
        }
    }

    prop::Prop* prop = scene.create_prop(shape, materials, name);

    // It is a annoying that this is done again in load_entities(),
    // but visibility information is already used when creating lights.
    // Should be improved at some point.
    if (visibility) {
        set_visibility(prop, *visibility);
    }

    return prop;
}

entity::Entity* Loader::load_extension(std::string const& type, json::Value const& extension_value,
                                       std::string const& name, Scene& scene) noexcept {
    if (auto p = extension_providers_.find(type); extension_providers_.end() != p) {
        entity::Entity* entity = p->second->create_extension(extension_value, scene,
                                                             resource_manager_);
        if (entity->is_extension()) {
            scene.add_extension(entity, name);
        }

        return entity;
    }

    return nullptr;
}

Scene::Shape* Loader::load_shape(json::Value const& shape_value) noexcept {
    if (std::string const type = json::read_string(shape_value, "type"); !type.empty()) {
        return shape(type, shape_value);
    }

    if (std::string const file = json::read_string(shape_value, "file"); !file.empty()) {
        return resource_manager_.load<shape::Shape>(file);
    }

    logging::error("Cannot create Shape: Neither shape nor type.");

    return nullptr;
}

Scene::Shape* Loader::shape(std::string const& type, json::Value const& shape_value) const
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
            if (!shape) {
                logging::error("Cannot create shape of type \"" + type + "\".");
            }

            return shape;

        } else {
            logging::error("Cannot create shape of type \"" + type + "\": Undefined type.");
        }
    }

    return nullptr;
}

void Loader::load_materials(json::Value const& materials_value, std::string_view mount_folder,
                            Local_materials const& local_materials, Scene& scene,
                            Materials& materials) const noexcept {
    if (!materials_value.IsArray()) {
        return;
    }

    materials.reserve(materials_value.Size());

    for (auto const& m : materials_value.GetArray()) {
        materials.push_back(load_material(m.GetString(), mount_folder, local_materials, scene));
    }
}

material::Material* Loader::load_material(std::string const& name, std::string_view mount_folder,
                                          Local_materials const& local_materials,
                                          Scene&                 scene) const noexcept {
    // First, check if we maybe already have cached the material.
    if (auto material = resource_manager_.get<material::Material>(name); material) {
        return material;
    }

    // Otherwise, see if it is among the locally defined materials.
    if (auto const material_node = local_materials.find(name);
        local_materials.end() != material_node) {
        void const* data = reinterpret_cast<void const*>(material_node->second);

        if (auto material = resource_manager_.load<material::Material>(name, data, mount_folder);
            material) {
            if (material->is_animated()) {
                scene.add_material(material);
            }

            return material;
        }
    }

    // Lastly, try loading the material from the filesystem.
    if (auto material = resource_manager_.load<material::Material>(name); material) {
        if (material->is_animated()) {
            scene.add_material(material);
        }

        return material;
    }

    logging::error("Loading \"" + name + "\": " /*+ e.what()*/ + ". Using fallback material.");

    return &fallback_material_;
}

}  // namespace scene
