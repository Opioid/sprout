#include "scene_loader.hpp"
#include "logging/logging.hpp"
#include "scene.hpp"
#include "scene/animation/animation.hpp"
#include "scene/animation/animation_loader.hpp"
#include "scene/entity/dummy.hpp"
#include "scene/entity/entity_extension_provider.hpp"
#include "scene/light/prop_light.hpp"
#include "scene/light/prop_image_light.hpp"
#include "scene/prop/prop.hpp"
#include "scene/shape/canopy.hpp"
#include "scene/shape/celestial_disk.hpp"
#include "scene/shape/disk.hpp"
#include "scene/shape/infinite_sphere.hpp"
#include "scene/shape/inverse_sphere.hpp"
#include "scene/shape/plane.hpp"
#include "scene/shape/sphere.hpp"
#include "scene/shape/triangle/triangle_bvh_preset.hpp"
#include "scene/shape/triangle/triangle_mesh.hpp"
#include "scene/shape/triangle/triangle_mesh_generator.hpp"
#include "scene/volume/volume.hpp"
#include "resource/resource_cache.inl"
#include "resource/resource_manager.inl"
#include "resource/resource_provider.inl"
#include "base/json/json.hpp"
#include "base/math/vector.inl"
#include "base/math/quaternion.inl"
#include "base/memory/variant_map.inl"
#include "base/thread/thread_pool.hpp"

namespace scene {

Loader::Loader(resource::Manager& manager, std::shared_ptr<material::Material> fallback_material) :
	resource_manager_(manager),
	canopy_(std::make_shared<shape::Canopy>()),
	celestial_disk_(std::make_shared<shape::Celestial_disk>()),
	disk_(std::make_shared<shape::Disk>()),
	infinite_sphere_(std::make_shared<shape::Infinite_sphere>()),
	inverse_sphere_(std::make_shared<shape::Inverse_sphere>()),
	plane_(std::make_shared<shape::Plane>()),
	sphere_(std::make_shared<shape::Sphere>()),
	fallback_material_(fallback_material) {}

Loader::~Loader() {}

void Loader::load(std::istream& stream, Scene& scene) {
	auto root = json::parse(stream);

	for (auto n = root->MemberBegin(); n != root->MemberEnd(); ++n) {
		const std::string node_name = n->name.GetString();
		const json::Value& node_value = n->value;

		if ("entities" == node_name) {
			load_entities(node_value, nullptr, scene);
		}
	}
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

void Loader::load_entities(const json::Value& entities_value,
						   entity::Entity* parent, Scene& scene) {
	if (!entities_value.IsArray()) {
		return;
	}

	for (auto e = entities_value.Begin(); e != entities_value.End(); ++e) {
		const json::Value::ConstMemberIterator type_node = e->FindMember("type");
		if (e->MemberEnd() == type_node) {
			continue;
		}

		std::string type_name = type_node->value.GetString();

		std::string name = json::read_string(*e, "name");

		entity::Entity* entity = nullptr;

		if ("Light" == type_name) {
			Prop* prop = load_prop(*e, name, scene);
			entity = prop;
			if (prop && prop->visible_in_reflection()) {
				load_light(*e, prop, scene);
			}
		} else if ("Prop" == type_name) {
			entity = load_prop(*e, name, scene);
		} else if ("Dummy" == type_name) {
			entity = scene.create_dummy();
		} else if ("Volume" == type_name) {
			entity = load_volume(*e, scene);
		} else {
			entity = load_extension(type_name, *e, name, scene);
		}

		if (!entity) {
			logging::error("Cannot create entity \"" + type_name + "\": Unknown type.");
			continue;
		}

		math::transformation transformation {
			math::float3_identity,
			math::float3(1.f, 1.f, 1.f),
			math::quaternion_identity
		};

		const json::Value* animation_value = nullptr;
		const json::Value* children = nullptr;

		for (auto n = e->MemberBegin(); n != e->MemberEnd(); ++n) {
			const std::string node_name = n->name.GetString();
			const json::Value& node_value = n->value;

			if ("transformation" == node_name) {
				json::read_transformation(node_value, transformation);
			} else if ("animation" == node_name) {
				animation_value = &node_value;
			} else if ("entities" == node_name) {
				children = &node_value;
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
		} else {
			entity->set_transformation(transformation);
		}
	}
}

Prop* Loader::load_prop(const json::Value& prop_value, const std::string& name, Scene& scene) {
	std::shared_ptr<shape::Shape> shape;
	material::Materials materials;
	bool visible_in_camera = true;
	bool visible_in_reflection = true;
	bool visible_in_shadow = true;
	bool open = false;

	for (auto n = prop_value.MemberBegin(); n != prop_value.MemberEnd(); ++n) {
		const std::string node_name = n->name.GetString();
		const json::Value& node_value = n->value;

		if ("shape" == node_name) {
			shape = load_shape(node_value);
		} else if ("materials" == node_name) {
			load_materials(node_value, scene, materials);
		} else if ("visibility" == node_name) {
			visible_in_camera	  = json::read_bool(node_value, "in_camera",     true);
			visible_in_reflection = json::read_bool(node_value, "in_reflection", true);
			visible_in_shadow     = json::read_bool(node_value, "in_shadow",     true);
		} else if ("open" == node_name) {
			open = json::read_bool(node_value);
		}
	}

	if (!shape) {
		return nullptr;
	}

	while (materials.size() < shape->num_parts()) {
		materials.push_back(fallback_material_);
	}

	Prop* prop = scene.create_prop(shape, name);

	prop->set_materials(materials);
	prop->set_visibility(visible_in_camera, visible_in_reflection, visible_in_shadow);
	prop->set_open(open);

	return prop;
}

void Loader::load_light(const json::Value& /*light_value*/, Prop* prop, Scene& scene) {
	const size_t num_parts = static_cast<size_t>(prop->shape()->num_parts());
	auto& materials = prop->materials();
	for (size_t i = 0, len = std::min(materials.size(), num_parts); i < len; ++i) {
		if (materials[i]->is_emissive()) {
			if (prop->shape()->is_analytical() && materials[i]->has_emission_map()) {
				scene.create_prop_image_light(prop, static_cast<uint32_t>(i));
			} else {
				scene.create_prop_light(prop, static_cast<uint32_t>(i));
			}
		}
	}
}

volume::Volume* Loader::load_volume(const json::Value& volume_value, Scene& scene) {
	math::float3 absorption(0.f, 0.f, 0.f);
	math::float3 scattering(0.f, 0.f, 0.f);

	for (auto n = volume_value.MemberBegin(); n != volume_value.MemberEnd(); ++n) {
		const std::string node_name = n->name.GetString();
		const json::Value& node_value = n->value;

		if ("absorption" == node_name) {
			absorption = json::read_float3(node_value);
		} else if ("scattering" == node_name) {
			scattering = json::read_float3(node_value);
		}
	}

	return scene.create_volume(absorption, scattering);
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
		try {
            memory::Variant_map options;

			std::string bvh_preset_value = json::read_string(shape_value, "bvh_preset");
			if ("fast" == bvh_preset_value) {
                options.insert("bvh_preset", shape::triangle::BVH_preset::Fast);
			} else if ("slow" == bvh_preset_value) {
                options.insert("bvh_preset", shape::triangle::BVH_preset::Slow);
			}

			return resource_manager_.load<shape::Shape>(file, options);
		} catch (const std::exception& e) {
			logging::error("Cannot load \"" + file + "\": " + e.what() + ".");
		}
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
	} else if ("Inverse_sphere" == type) {
		return inverse_sphere_;
	} else if ("Plane" == type) {
		return plane_;
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

	for (auto m = materials_value.Begin(); m != materials_value.End(); ++m) {
		try {
			bool was_cached;
			auto material = resource_manager_.load<material::Material>(m->GetString(),
																	   memory::Variant_map(),
																	   was_cached);

			if (material->is_animated() && !was_cached) {
				scene.add_material(material);
			}

			materials.push_back(material);
		} catch (const std::exception& e) {
			materials.push_back(fallback_material_);

			logging::error("Loading \"" + std::string(m->GetString()) + "\": " +
						   e.what() + ". Using fallback material.");
		}
	}
}

}
