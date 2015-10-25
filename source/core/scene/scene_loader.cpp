#include "scene_loader.hpp"
#include "scene.hpp"
#include "scene/animation/animation.hpp"
#include "scene/animation/animation_loader.hpp"
#include "scene/entity/dummy.hpp"
#include "scene/light/prop_light.hpp"
#include "scene/light/prop_image_light.hpp"
#include "scene/prop/prop.hpp"
#include "scene/shape/canopy.hpp"
#include "scene/shape/celestial_disk.hpp"
#include "scene/shape/disk.hpp"
#include "scene/shape/inverse_sphere.hpp"
#include "scene/shape/plane.hpp"
#include "scene/shape/sphere.hpp"
#include "scene/shape/triangle/triangle_mesh.hpp"
#include "scene/material/material_sample_cache.inl"
#include "resource/resource_cache.inl"
#include "resource/resource_provider.inl"
#include "base/json/json.hpp"
#include "base/math/vector.inl"
#include "base/math/quaternion.inl"
#include <iostream>

namespace scene {

Loader::Loader(file::System& file_system, uint32_t num_workers) :
	canopy_(std::make_shared<shape::Canopy>()),
	celestial_disk_(std::make_shared<shape::Celestial_disk>()),
	disk_(std::make_shared<shape::Disk>()),
	inverse_sphere_(std::make_shared<shape::Inverse_sphere>()),
	plane_(std::make_shared<shape::Plane>()),
	sphere_(std::make_shared<shape::Sphere>()),
	mesh_provider_(file_system),
	mesh_cache_(mesh_provider_),
	texture_provider_(file_system),
	texture_cache_(texture_provider_),
	material_provider_(file_system, texture_cache_, num_workers),
	material_cache_(material_provider_) {}

Loader::~Loader() {}

void Loader::load(std::istream& stream, Scene& scene) {
	auto root = json::parse(stream);

	for (auto n = root->MemberBegin(); n != root->MemberEnd(); ++n) {
		const std::string node_name = n->name.GetString();
		const rapidjson::Value& node_value = n->value;

		if ("entities" == node_name) {
			load_entities(node_value, nullptr, scene);
		}
	}
}

void Loader::load_entities(const rapidjson::Value& entities_value, entity::Entity* parent, Scene& scene) {
	if (!entities_value.IsArray()) {
		return;
	}

	for (auto e = entities_value.Begin(); e != entities_value.End(); ++e) {
		const rapidjson::Value::ConstMemberIterator type_node = e->FindMember("type");
		if (e->MemberEnd() == type_node) {
			continue;
		}

		std::string type_name = type_node->value.GetString();

		entity::Entity* entity = nullptr;

		if ("Light" == type_name) {
			Prop* prop = load_prop(*e, scene);
			entity = prop;
			if (prop && prop->secondary_visibility()) {
				load_light(*e, prop, scene);
			}
		} else if ("Prop" == type_name) {
			entity = load_prop(*e, scene);
		} else if ("Dummy" == type_name) {
			entity = scene.create_dummy();
		}

		if (!entity) {
			continue;
		}

		math::transformation transformation{
			math::float3::identity,
			math::float3(1.f, 1.f, 1.f),
			math::quaternion::identity
		};

		const rapidjson::Value* animation_value = nullptr;
		const rapidjson::Value* children = nullptr;

		for (auto n = e->MemberBegin(); n != e->MemberEnd(); ++n) {
			const std::string node_name = n->name.GetString();
			const rapidjson::Value& node_value = n->value;

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

Prop* Loader::load_prop(const rapidjson::Value& prop_value, Scene& scene) {
	std::shared_ptr<shape::Shape> shape;
	material::Materials materials;
	bool primary_visibility = true;
	bool secondary_visibility = true;

	for (auto n = prop_value.MemberBegin(); n != prop_value.MemberEnd(); ++n) {
		const std::string node_name = n->name.GetString();
		const rapidjson::Value& node_value = n->value;

		if ("shape" == node_name) {
			shape = load_shape(node_value);
		} else if ("materials" == node_name) {
			load_materials(node_value, materials);
		} else if ("visibility" == node_name) {
			primary_visibility   = json::read_bool(node_value, "primary",   true);
			secondary_visibility = json::read_bool(node_value, "secondary", true);
		}
	}

	if (!shape) {
		return nullptr;
	}

	while (materials.size() < shape->num_parts()) {
		materials.push_back(material_provider_.fallback_material());
	}

	Prop* prop = scene.create_prop(shape, materials);

	prop->set_visibility(primary_visibility, secondary_visibility);

	return prop;
}

light::Light* Loader::load_light(const rapidjson::Value& /*light_value*/, Prop* prop, Scene& scene) {
	light::Prop_light* light = nullptr;

	if (prop->shape()->is_analytical() && prop->has_emission_mapped_material()) {
		light = scene.create_prop_image_light(prop);
	} else {
		light = scene.create_prop_light(prop);
	}

	return light;
}

std::shared_ptr<shape::Shape> Loader::load_shape(const rapidjson::Value& shape_value) {
	std::string type = json::read_string(shape_value, "type");
	if (!type.empty()) {
		return shape(type);
	}

	std::string file = json::read_string(shape_value, "file");
	if (!file.empty()) {
		try {
			return mesh_cache_.load(file);
		} catch (const std::exception& e) {
			std::cout << "Cannot load \"" << file << "\": " << e.what() << std::endl;
		}
	}

	return nullptr;
}

std::shared_ptr<shape::Shape> Loader::shape(const std::string& type) const {
	if ("Canopy" == type) {
		return canopy_;
	} else if ("Celestial_disk" == type) {
		return celestial_disk_;
	} else if ("Disk" == type) {
		return disk_;
	} else if ("Inverse_sphere" == type) {
		return inverse_sphere_;
	} else if ("Plane" == type) {
		return plane_;
	} else if ("Sphere" == type) {
		return sphere_;
	}

	return nullptr;
}

void Loader::load_materials(const rapidjson::Value& materials_value, material::Materials& materials) {
	if (!materials_value.IsArray()) {
		return;
	}

	materials.reserve(materials_value.Size());

	for (auto m = materials_value.Begin(); m != materials_value.End(); ++m) {
		try {
			auto material = material_cache_.load(m->GetString());

			materials.push_back(material);
		} catch (const std::exception& e) {
			materials.push_back(material_provider_.fallback_material());

			std::cout << "Loading \"" << m->GetString() << "\": " << e.what() << ". Using fallback material. " << std::endl;
		}
	}
}


}
