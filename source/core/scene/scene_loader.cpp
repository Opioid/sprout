#include "scene_loader.hpp"
#include "logging/logging.hpp"
#include "image/texture/texture_provider.hpp"
#include "scene.hpp"
#include "animation/animation.hpp"
#include "animation/animation_loader.hpp"
#include "entity/dummy.hpp"
#include "entity/entity_extension_provider.hpp"
#include "light/prop_light.hpp"
#include "light/prop_image_light.hpp"
#include "prop/prop.hpp"
#include "shape/box.hpp"
#include "shape/canopy.hpp"
#include "shape/celestial_disk.hpp"
#include "shape/disk.hpp"
#include "shape/infinite_sphere.hpp"
#include "shape/plane.hpp"
#include "shape/rectangle.hpp"
#include "shape/sphere.hpp"
#include "shape/triangle/triangle_mesh.hpp"
#include "shape/triangle/triangle_mesh_generator.hpp"
#include "resource/resource_manager.inl"
#include "base/json/json.hpp"
#include "base/math/vector3.inl"
#include "base/math/quaternion.inl"
#include "base/memory/variant_map.inl"
#include "base/string/string.hpp"
#include "base/thread/thread_pool.hpp"

namespace scene {

Loader::Loader(resource::Manager& manager, const Material_ptr& fallback_material) :
	resource_manager_(manager),
	box_(std::make_shared<shape::Box>()),
	canopy_(std::make_shared<shape::Canopy>()),
	celestial_disk_(std::make_shared<shape::Celestial_disk>()),
	disk_(std::make_shared<shape::Disk>()),
	infinite_sphere_(std::make_shared<shape::Infinite_sphere>()),
	plane_(std::make_shared<shape::Plane>()),
	rectangle_(std::make_shared<shape::Rectangle>()),
	sphere_(std::make_shared<shape::Sphere>()),
	fallback_material_(fallback_material) {}

Loader::~Loader() {}

bool Loader::load(std::string const& filename, std::string const& take_name, Scene& scene) {
	bool success = true;

	try {
		auto& filesystem = resource_manager_.filesystem();

		std::string const take_mount_folder = string::parent_directory(take_name);
		filesystem.push_mount(take_mount_folder);

		std::string resolved_name;
		auto stream_pointer = filesystem.read_stream(filename, resolved_name);

		filesystem.pop_mount();

		mount_folder_ = string::parent_directory(resolved_name);

		auto root = json::parse(*stream_pointer);

		if (auto const materials_node = root->FindMember("materials");
			root->MemberEnd() != materials_node) {
			read_materials(materials_node->value);
		}

		filesystem.push_mount(mount_folder_);

		for (auto& n : root->GetObject()) {
			if ("entities" == n.name) {
				load_entities(n.value, nullptr, scene);
			}
		}

		filesystem.pop_mount();

		scene.finish();
	} catch (std::exception& e) {
		success = false;
		logging::error("Scene \"" + filename + "\" could not be loaded: " + e.what() + ".");
	}

	resource_manager_.thread_pool().wait_async();

	return success;
}

void Loader::register_extension_provider(std::string const& name,
										 entity::Extension_provider* provider) {
	extension_providers_[name] = provider;
}

void Loader::register_mesh_generator(std::string const& name,
									 shape::triangle::Generator* generator) {
    mesh_generators_[name] = generator;
}

std::shared_ptr<shape::Shape> Loader::box() {
	return box_;
}

std::shared_ptr<shape::Shape> Loader::canopy() {
	return canopy_;
}

std::shared_ptr<shape::Shape> Loader::celestial_disk() {
	return celestial_disk_;
}

size_t Loader::num_bytes() const {
	return 0;
}

void Loader::read_materials(json::Value const& materials_value) {
	if (!materials_value.IsArray()) {
		return;
	}

	for (auto m = materials_value.Begin(); m != materials_value.End(); ++m) {
		json::Value::ConstMemberIterator const name_node = m->FindMember("name");
		if (m->MemberEnd() == name_node) {
			continue;
		}

		std::string const name = name_node->value.GetString();

		local_materials_[name] = m;
	}
}

void Loader::load_entities(json::Value const& entities_value,
						   entity::Entity* parent, Scene& scene) {
	if (!entities_value.IsArray()) {
		return;
	}

	for (auto& e : entities_value.GetArray()) {
		auto const type_node = e.FindMember("type");
		if (e.MemberEnd() == type_node) {
			continue;
		}

		std::string const type_name = type_node->value.GetString();

		std::string const name = json::read_string(e, "name");

		entity::Entity* entity = nullptr;

		try {
			if ("Light" == type_name) {
				prop::Prop* prop = load_prop(e, name, scene);
				entity = prop;
				if (prop && prop->visible_in_reflection()) {
					load_light(e, prop, scene);
				}
			} else if ("Prop" == type_name) {
				entity = load_prop(e, name, scene);
			} else if ("Dummy" == type_name) {
				entity = scene.create_dummy();
			} else {
				entity = load_extension(type_name, e, name, scene);
			}
		} catch (const std::exception& e) {
			logging::error("Cannot create entity \"" + type_name + "\": " + e.what() + ".");
			continue;
		}

		if (!entity) {
			continue;
		}

		math::Transformation transformation {
			float3::identity(),
			float3(1.f),
			math::quaternion::identity()
		};

		json::Value const* animation_value = nullptr;
		json::Value const* children = nullptr;
		json::Value const* visibility = nullptr;

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
			load_entities(*children, entity, scene);
		}

		if (parent) {
			parent->attach(entity);
		}

		if (animation_value) {
			if (auto animation = animation::load(*animation_value, transformation); animation) {
				scene.add_animation(animation);
				scene.create_animation_stage(entity, animation.get());
			}
		} /*else*/ {
			entity->set_transformation(transformation);
		}

		if (visibility) {
			set_visibility(entity, *visibility);
		}
	}
}

void Loader::set_visibility(entity::Entity* entity, json::Value const& visibility_value) {
	bool in_camera	   = true;
	bool in_reflection = true;
	bool in_shadow	   = true;
//	bool propagate	   = false;

	for (auto& n : visibility_value.GetObject()) {
		if ("in_camera" == n.name) {
			in_camera = json::read_bool(n.value);
		} else if ("in_reflection" == n.name) {
			in_reflection = json::read_bool(n.value);
		} else if ("in_shadow" == n.name) {
			in_shadow = json::read_bool(n.value);
		} /*else if ("propagate" == n.name) {
				propagate = json::read_bool(n.value);
		}*/
	}

	entity->set_visibility(in_camera, in_reflection, in_shadow);
//	entity->set_propagate_visibility(propagate);
}

prop::Prop* Loader::load_prop(json::Value const& prop_value,
							  std::string const& name, Scene& scene) {
	std::shared_ptr<shape::Shape> shape;
	Materials materials;
	json::Value const* visibility = nullptr;

	for (auto& n : prop_value.GetObject()) {
		if ("shape" == n.name) {
			shape = load_shape(n.value);
		} else if ("materials" == n.name) {
			load_materials(n.value, scene, materials);
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
			materials.push_back(fallback_material_);
		}
	}

	prop::Prop* prop = scene.create_prop(shape, materials, name);

	// Bit annoying that this is done again in load_entities(),
	// but visibility information is already used when creating lights.
	// Should be improved at some point.
	if (visibility) {
		set_visibility(prop, *visibility);
	}

	return prop;
}

void Loader::load_light(json::Value const& /*light_value*/, prop::Prop* prop, Scene& scene) {
	for (uint32_t i = 0, len = prop->shape()->num_parts(); i < len; ++i) {
		if (auto const material = prop->material(i); material->is_emissive()) {
			if (prop->shape()->is_analytical() && material->has_emission_map()) {
				scene.create_prop_image_light(prop, i);
			} else {
				scene.create_prop_light(prop, i);
			}
		}
	}
}

entity::Entity* Loader::load_extension(std::string const& type, json::Value const& extension_value,
									   std::string const& name, Scene& scene) {
	if (auto p = extension_providers_.find(type); extension_providers_.end() != p) {
		entity::Entity* entity = p->second->create_extension(extension_value, scene,
															 resource_manager_);
		scene.add_extension(entity, name);

		return entity;
	}

	return nullptr;
}

std::shared_ptr<shape::Shape> Loader::load_shape(json::Value const& shape_value) {
	if (std::string const type = json::read_string(shape_value, "type"); !type.empty()) {
        return shape(type, shape_value);
	}

	if (std::string const file = json::read_string(shape_value, "file"); !file.empty()) {
		return resource_manager_.load<shape::Shape>(file);
	}

	return nullptr;
}

std::shared_ptr<shape::Shape> Loader::shape(std::string const& type,
											json::Value const& shape_value) const {
	if ("Box" == type) {
		return box_;
	} else if ("Canopy" == type) {
		return canopy_;
	} else if ("Celestial_disk" == type) {
		return celestial_disk_;
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
			try {
				return g->second->create_mesh(shape_value, resource_manager_);
			} catch (const std::exception& e) {
				logging::error("Cannot create shape of type \"" + type + "\": " + e.what() + ".");
			}
		} else {
			logging::error("Cannot create shape of type \"" + type + "\": Undefined type.");
		}
	}

	return nullptr;
}

void Loader::load_materials(json::Value const& materials_value, Scene& scene,
							Materials& materials) {
	if (!materials_value.IsArray()) {
		return;
	}

	materials.reserve(materials_value.Size());

	for (auto& m : materials_value.GetArray()) {
		materials.push_back(load_material(m.GetString(), scene));
	}
}

Material_ptr Loader::load_material(std::string const& name, Scene& scene) {
	// First, check if we maybe already have cached the material.
	if (auto material = resource_manager_.get<material::Material>(name); material) {
		return material;
	}

	try {
		// Otherwise, see if it is among the locally defined materials.
		if (auto const material_node = local_materials_.find(name);
			local_materials_.end() != material_node) {
			void const* data = reinterpret_cast<void const*>(material_node->second);

			auto material = resource_manager_.load<material::Material>(name, data, mount_folder_);

			if (material->is_animated()) {
				scene.add_material(material);
			}

			return material;
		}

		// Lastly, try loading the material from the filesystem.
		auto material = resource_manager_.load<material::Material>(name);

		if (material->is_animated()) {
			scene.add_material(material);
		}

		return material;
	} catch (const std::exception& e) {
		logging::error("Loading \"" + name + "\": " + e.what() + ". Using fallback material.");
	}

	return fallback_material_;
}

}
