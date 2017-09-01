#include "scene_loader.hpp"
#include "logging/logging.hpp"
#include "image/texture/texture_provider.hpp"
#include "scene.hpp"
#include "prop.hpp"
#include "animation/animation.hpp"
#include "animation/animation_loader.hpp"
#include "entity/dummy.hpp"
#include "entity/entity_extension_provider.hpp"
#include "light/prop_light.hpp"
#include "light/prop_image_light.hpp"
#include "shape/canopy.hpp"
#include "shape/celestial_disk.hpp"
#include "shape/disk.hpp"
#include "shape/infinite_sphere.hpp"
#include "shape/plane.hpp"
#include "shape/rectangle.hpp"
#include "shape/sphere.hpp"
#include "shape/triangle/triangle_bvh_preset.hpp"
#include "shape/triangle/triangle_mesh.hpp"
#include "shape/triangle/triangle_mesh_generator.hpp"
#include "volume/volume.hpp"
#include "resource/resource_cache.inl"
#include "resource/resource_manager.inl"
#include "resource/resource_provider.inl"
#include "base/json/json.hpp"
#include "base/math/vector3.inl"
#include "base/math/quaternion.inl"
#include "base/memory/variant_map.inl"
#include "base/string/string.hpp"
#include "base/thread/thread_pool.hpp"

#include <iostream>

namespace scene {

Loader::Loader(resource::Manager& manager, material::Material_ptr fallback_material) :
	resource_manager_(manager),
	canopy_(std::make_shared<shape::Canopy>()),
	celestial_disk_(std::make_shared<shape::Celestial_disk>()),
	disk_(std::make_shared<shape::Disk>()),
	infinite_sphere_(std::make_shared<shape::Infinite_sphere>()),
	plane_(std::make_shared<shape::Plane>()),
	rectangle_(std::make_shared<shape::Rectangle>()),
	sphere_(std::make_shared<shape::Sphere>()),
	fallback_material_(fallback_material) {}

Loader::~Loader() {}

void Loader::load(const std::string& filename, Scene& scene) {
	std::string resolved_name;
	auto stream_pointer = resource_manager_.file_system().read_stream(filename, resolved_name);

	mount_folder_ = string::parent_directory(resolved_name);

	auto root = json::parse(*stream_pointer);

	const json::Value::ConstMemberIterator materials_node = root->FindMember("materials");
	if (root->MemberEnd() != materials_node) {
	//	local_materials_ = &materials_node->value;
//		std::string mount_folder = string::parent_directory(resolved_name);
		read_materials(materials_node->value);
	}

	resource_manager_.file_system().push_mount(mount_folder_);

	for (auto& n : root->GetObject()) {
		if ("entities" == n.name) {
			load_entities(n.value, nullptr, scene);
		}
	}

	resource_manager_.file_system().pop_mount();

	scene.finish();

	resource_manager_.thread_pool().wait_async();
}

void Loader::register_extension_provider(const std::string& name,
										 entity::Extension_provider* provider) {
	extension_providers_[name] = provider;
}

void Loader::register_mesh_generator(const std::string& name,
									 shape::triangle::Generator* generator) {
    mesh_generators_[name] = generator;
}

std::shared_ptr<shape::Shape> Loader::canopy() {
	return canopy_;
}

std::shared_ptr<shape::Shape> Loader::celestial_disk() {
	return celestial_disk_;
}

void Loader::read_materials(const json::Value& materials_value) {
	if (!materials_value.IsArray()) {
		return;
	}

	for (auto m = materials_value.Begin(); m != materials_value.End(); ++m) {
		const json::Value::ConstMemberIterator name_node = m->FindMember("name");
		if (m->MemberEnd() == name_node) {
			continue;
		}

		const std::string name = name_node->value.GetString();

		local_materials_[name] = m;
	}
}

void Loader::load_entities(const json::Value& entities_value,
						   entity::Entity* parent, Scene& scene) {
	if (!entities_value.IsArray()) {
		return;
	}

	for (auto& e : entities_value.GetArray()) {
		const auto type_node = e.FindMember("type");
		if (e.MemberEnd() == type_node) {
			continue;
		}

		std::string type_name = type_node->value.GetString();

		std::string name = json::read_string(e, "name");

		entity::Entity* entity = nullptr;

		try {
			if ("Light" == type_name) {
				scene::Prop* prop = load_prop(e, name, scene);
				entity = prop;
				if (prop && prop->visible_in_reflection()) {
					load_light(e, prop, scene);
				}
			} else if ("Prop" == type_name) {
				entity = load_prop(e, name, scene);
			} else if ("Dummy" == type_name) {
				entity = scene.create_dummy();
			} else if ("Volume" == type_name) {
				entity = load_volume(e, scene);
			} else {
				entity = load_extension(type_name, e, name, scene);
			}
		} catch (const std::exception& e) {
			logging::error("Cannot create entity \"" + type_name + "\": " + e.what() + ".");
			continue;
		}

		if (!entity) {
			logging::error("Cannot create entity \"" + type_name + "\": Unknown type.");
			continue;
		}

		math::Transformation transformation {
			float3::identity(),
			float3(1.f),
			math::quaternion::identity()
		};

		const json::Value* animation_value = nullptr;
		const json::Value* children = nullptr;
		const json::Value* visibility = nullptr;

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
			auto animation = animation::load(*animation_value, transformation);
			if (animation) {
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

void Loader::set_visibility(entity::Entity* entity, const json::Value& visibility_value) {
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

Prop* Loader::load_prop(const json::Value& prop_value, const std::string& name, Scene& scene) {
	std::shared_ptr<shape::Shape> shape;
	material::Materials materials;
	const json::Value* visibility = nullptr;
	bool open = false;

	for (auto& n : prop_value.GetObject()) {
		if ("shape" == n.name) {
			shape = load_shape(n.value);
		} else if ("materials" == n.name) {
			load_materials(n.value, scene, materials);
		} else if ("visibility" == n.name) {
			visibility = &n.value;
		} else if ("open" == n.name) {
			open = json::read_bool(n.value);
		}
	}

	if (!shape) {
		return nullptr;
	}

	while (materials.size() < shape->num_parts()) {
		materials.push_back(fallback_material_);
	}

	Prop* prop = scene.create_prop(shape, materials, name);

	// Bit annoying that this is done again in load_entities(),
	// but visibility information is already used when creating lights.
	// Should be improved at some point.
	if (visibility) {
		set_visibility(prop, *visibility);
	}

	prop->set_open(open);

	return prop;
}

void Loader::load_light(const json::Value& /*light_value*/, Prop* prop, Scene& scene) {
	for (uint32_t i = 0, len = prop->shape()->num_parts(); i < len; ++i) {
		const auto material = prop->material(i);
		if (material->is_emissive()) {
			if (prop->shape()->is_analytical() && material->has_emission_map()) {
				scene.create_prop_image_light(prop, i);
			} else {
				scene.create_prop_light(prop, i);
			}
		}
	}
}

volume::Volume* Loader::load_volume(const json::Value& volume_value, Scene& scene) {
	std::string shape_type;
	std::string file;
	const json::Value* parameters_value = nullptr;

	for (auto& n : volume_value.GetObject()) {
		if ("shape" == n.name) {
			shape_type = json::read_string(n.value, "type");
			file = json::read_string(n.value, "file");
		} else if ("parameters" == n.name) {
			parameters_value = &n.value;
		}
	}

	volume::Volume* volume = nullptr;

	if ("Homogenous" == shape_type) {
		volume = scene.create_homogenous_volume();
	} else if ("Height" == shape_type) {
		volume = scene.create_height_volume();
	} else if (!file.empty()) {
		memory::Variant_map options;
		options.set("usage", image::texture::Provider::Usage::Mask);
		auto grid = resource_manager_.load<image::texture::Texture>(file, options);
		volume = scene.create_grid_volume(grid);
	}

	if (!volume) {
		throw std::runtime_error("Cannot create shape \"" + shape_type + "\": Unknown type.");
	}

	if (parameters_value) {
		volume->set_parameters(*parameters_value);
	}

	return volume;
}

entity::Entity* Loader::load_extension(const std::string& type,
									   const json::Value& extension_value,
									   const std::string& name,
									   Scene& scene) {
	auto p = extension_providers_.find(type);
	if (extension_providers_.end() != p) {
		entity::Entity* entity = p->second->create_extension(extension_value, scene,
															 resource_manager_);
		scene.add_extension(entity, name);

		return entity;
	}

	return nullptr;
}

std::shared_ptr<shape::Shape> Loader::load_shape(const json::Value& shape_value) {
	std::string type = json::read_string(shape_value, "type");
	if (!type.empty()) {
        return shape(type, shape_value);
	}

	std::string file = json::read_string(shape_value, "file");
	if (!file.empty()) {
		memory::Variant_map options;

		std::string bvh_preset_value = json::read_string(shape_value, "bvh_preset");

		if ("fast" == bvh_preset_value) {
			options.set("bvh_preset", shape::triangle::BVH_preset::Fast);
		} else if ("slow" == bvh_preset_value) {
			options.set("bvh_preset", shape::triangle::BVH_preset::Slow);
		}

		return resource_manager_.load<shape::Shape>(file, options);
	}

	return nullptr;
}

std::shared_ptr<shape::Shape> Loader::shape(const std::string& type,
											const json::Value& shape_value) const {
	if ("Canopy" == type) {
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
        auto g = mesh_generators_.find(type);
        if (mesh_generators_.end() != g) {
			try {
				return g->second->create_mesh(shape_value, resource_manager_);
			} catch (const std::exception& e) {
				logging::error("Cannot create shape \"" + type + "\": " + e.what() + ".");
			}
		} else {
			logging::error("Cannot create shape \"" + type + "\": Unknown type.");
		}
	}

	return nullptr;
}

void Loader::load_materials(const json::Value& materials_value, Scene& scene,
							material::Materials& materials) {
	if (!materials_value.IsArray()) {
		return;
	}

	materials.reserve(materials_value.Size());

	for (auto& m : materials_value.GetArray()) {
		/*
		try {
			bool was_cached;
			auto material = resource_manager_.load<material::Material>(m.GetString(),
																	   memory::Variant_map(),
																	   was_cached);

			if (material->is_animated() && !was_cached) {
				scene.add_material(material);
			}

			materials.push_back(material);
		} catch (const std::exception& e) {
			materials.push_back(fallback_material_);

			logging::error("Loading \"" + std::string(m.GetString()) + "\": " +
						   e.what() + ". Using fallback material.");
		}
		*/

		materials.push_back(load_material(m.GetString(), scene));
	}
}

material::Material_ptr Loader::load_material(const std::string& name, Scene& scene) {
	// First, check if we maybe already have cached the material.
	auto material = resource_manager_.get<material::Material>(name, memory::Variant_map());
	if (material) {
		return material;
	}

	try {
		// Otherwise, see if it is among the locally defined materials.
		const auto material_node = local_materials_.find(name);
		if (local_materials_.end() != material_node) {
			const void* data = reinterpret_cast<const void*>(material_node->second);

			material = resource_manager_.load<material::Material>(name, data, mount_folder_,
																  memory::Variant_map());

			if (material->is_animated()) {
				scene.add_material(material);
			}

			return material;
		}

		// Lastly, try loading the material from the filesystem.
		bool was_cached;
		material = resource_manager_.load<material::Material>(name, memory::Variant_map(),
															  was_cached);

		// Technically, the was_cached business is no longer needed,
		// because it is handled as a special case at the beginning of this function now.
		if (material->is_animated() && !was_cached) {
			scene.add_material(material);
		}

		return material;
	} catch (const std::exception& e) {
		logging::error("Loading \"" + name + "\": " + e.what() + ". Using fallback material.");
	}

	return fallback_material_;
}

}
