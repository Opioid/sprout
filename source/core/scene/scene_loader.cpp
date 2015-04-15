#include "scene_loader.hpp"
#include "scene.hpp"
#include "scene/surrounding/uniform_surrounding.hpp"
#include "scene/surrounding/sphere_surrounding.hpp"
#include "scene/light/shape_light.hpp"
#include "scene/shape/plane.hpp"
#include "scene/shape/sphere.hpp"
#include "scene/shape/celestial_disk.hpp"
#include "scene/shape/triangle/triangle_mesh.hpp"
#include "scene/material/material_sample_cache.inl"
#include "image/image4.hpp"
#include "resource/resource_cache.inl"
#include "base/json/json.hpp"
#include "base/math/vector.inl"
#include <fstream>
#include <iostream>

namespace scene {

Loader::Loader(uint32_t num_workers) :
	plane_(std::make_shared<shape::Plane>()),
	sphere_(std::make_shared<shape::Sphere>()),
	celestial_disk_(std::make_shared<shape::Celestial_disk>()),
	mesh_cache_(mesh_provider_),
	image_cache_(image_provider_),
	material_provider_(num_workers),
	material_cache_(material_provider_) {}

Loader::~Loader() {}

void Loader::load(const std::string& filename, Scene& scene) {
	std::ifstream stream(filename, std::ios::binary);
	if (!stream) {
		throw std::runtime_error("Could not open file");
	}

	auto root = json::parse(stream);

	for (auto n = root->MemberBegin(); n != root->MemberEnd(); ++n) {
		const std::string node_name = n->name.GetString();
		const rapidjson::Value& node_value = n->value;

		if ("surrounding" == node_name) {
			surrounding::Surrounding* surrounding = load_surrounding(node_value);
			scene.set_surrounding(surrounding);
		} else if ("entities" == node_name) {
			load_entities(node_value, scene);
		}
	}

	scene.compile();
}

surrounding::Surrounding* Loader::load_surrounding(const rapidjson::Value& surrounding_value) {
	std::string type_name = json::read_string(surrounding_value, "type");

	if ("Uniform" == type_name) {
		math::float3 energy = json::read_float3(surrounding_value, "color");
		return new surrounding::Uniform(energy);
	} else if ("Textured" == type_name) {
		const rapidjson::Value::ConstMemberIterator texture_node = surrounding_value.FindMember("texture");
		if (surrounding_value.MemberEnd() != texture_node) {
			std::string file = json::read_string(texture_node->value, "file");
			if (!file.empty()) {
				auto image = image_cache_.load(file);
				return new surrounding::Sphere(image);
			}
		}
	}

	return nullptr;
}

void Loader::load_entities(const rapidjson::Value& entities_value, Scene& scene) {
	if (!entities_value.IsArray()) {
		return;
	}

	for (auto e = entities_value.Begin(); e != entities_value.End(); ++e) {
		const rapidjson::Value::ConstMemberIterator type_node = e->FindMember("type");
		if (e->MemberEnd() == type_node) {
			continue;
		}

		std::string type_name = type_node->value.GetString();

		Entity* entity = nullptr;

		if ("Light" == type_name) {
			entity = load_light(*e, scene);
		} else if ("Prop" == type_name) {
			entity = load_prop(*e, scene);
		}

		if (!entity) {
			continue;
		}

		math::float3 position = math::float3::identity;
		math::float3 scale = math::float3(1.f, 1.f, 1.f);
		math::quaternion rotation = math::quaternion::identity;

		for (auto n = e->MemberBegin(); n != e->MemberEnd(); ++n) {
			const std::string node_name = n->name.GetString();
			const rapidjson::Value& node_value = n->value;

			if ("position" == node_name) {
				position = json::read_float3(node_value);
			} else if ("scale" == node_name) {
				scale = json::read_float3(node_value);
			} else if ("rotation" == node_name) {
				rotation = json::read_local_rotation(node_value);
			}
		}

		entity->set_transformation(position, scale, rotation);
	}
}

Prop* Loader::load_prop(const rapidjson::Value& prop_value, Scene& scene) {
	std::shared_ptr<shape::Shape> shape;
	Prop::Materials materials;

	for (auto n = prop_value.MemberBegin(); n != prop_value.MemberEnd(); ++n) {
		const std::string node_name = n->name.GetString();
		const rapidjson::Value& node_value = n->value;

		if ("shape" == node_name) {
			shape = load_shape(node_value);
		} else if ("materials" == node_name) {
			load_materials(node_value, materials);
		}
	}

	if (!shape || materials.empty()) {
		return nullptr;
	}

	Prop* prop = scene.create_prop();

	prop->init(shape, materials);

	return prop;
}

light::Light* Loader::load_light(const rapidjson::Value& light_value, Scene& scene) {
	std::shared_ptr<shape::Shape> shape;
	math::float3 color = math::float3::identity;
	float lumen = 1.f;

	for (auto n = light_value.MemberBegin(); n != light_value.MemberEnd(); ++n) {
		const std::string node_name = n->name.GetString();
		const rapidjson::Value& node_value = n->value;

		if ("shape" == node_name) {
			shape = load_shape(node_value);
		} else if ("color" == node_name) {
			color = json::read_float3(node_value);
		} else if ("lumen" == node_name) {
			lumen = json::read_float(node_value);
		}
	}

	if (!shape) {
		return nullptr;
	}

	light::Shape_light* light = scene.create_shape_light();

	light->init(shape);
	light->set_color(color);
	light->set_lumen(lumen);

	return light;
}

std::shared_ptr<shape::Shape> Loader::load_shape(const rapidjson::Value& shape_value) {
	std::string type = json::read_string(shape_value, "type");
	if (!type.empty()) {
		return shape(type);
	}

	std::string file = json::read_string(shape_value, "file");
	if (!file.empty()) {
		return mesh_cache_.load(file);
	}

	return nullptr;
}

std::shared_ptr<shape::Shape> Loader::shape(const std::string& type) const {
	if ("Plane" == type) {
		return plane_;
	} else if ("Sphere" == type) {
		return sphere_;
	} else if ("Celestial_disk" == type) {
		return celestial_disk_;
	}

	return nullptr;
}

void Loader::load_materials(const rapidjson::Value& materials_value, Prop::Materials& materials) {
	if (!materials_value.IsArray()) {
		return;
	}

	materials.reserve(materials_value.Size());

	for (auto m = materials_value.Begin(); m != materials_value.End(); ++m) {
		try {
			auto material = material_cache_.load(m->GetString());

			if (material) {
				materials.push_back(material);
			} else {
				materials.push_back(material_provider_.fallback_material());
			}
		} catch (const std::exception& e) {
			materials.push_back(material_provider_.fallback_material());
		}
	}
}


}
