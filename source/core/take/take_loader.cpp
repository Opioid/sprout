#include "take_loader.hpp"
#include "take.hpp"
#include "rendering/film/unfiltered.hpp"
#include "rendering/integrator/surface/ao.hpp"
#include "rendering/sampler/random_sampler.hpp"
#include "scene/camera/perspective_camera.hpp"
#include "base/math/math.hpp"
#include "base/math/vector.inl"
#include "base/math/matrix.inl"
#include "base/math/quaternion.inl"
#include "base/json/json.hpp"
#include <fstream>
#include <iostream>

namespace take {

std::shared_ptr<Take> Loader::load(const std::string& filename) {
	std::ifstream stream(filename, std::ios::binary);
	if (!stream) {
		return nullptr;
	}

	auto root = json::parse(stream);
	if (!root) {
		return nullptr;
	}

	auto take = std::make_shared<Take>();

	for (auto n = root->MemberBegin(); n != root->MemberEnd(); ++n) {
		const std::string node_name = n->name.GetString();
		const rapidjson::Value& node_value = n->value;

		if ("scene" == node_name) {
			take->scene = node_value.GetString();
		} else if ("camera" == node_name) {
			take->context.camera = load_camera(node_value);
		} else if ("sampler" == node_name) {
		} else if ("integrator" == node_name) {
			take->surface_integrator_factory = load_surface_integrator_factory(node_value);
		}
	}

	if (!take->sampler) {
		take->sampler = std::make_shared<rendering::sampler::Random>(1, take->rng);
	}

	if (!take->context.camera) {
		return nullptr;
	}

	return take;
}

std::shared_ptr<camera::Camera> Loader::load_camera(const rapidjson::Value& camera_value) const {
	std::string type_name = "Perspective";
	const rapidjson::Value* type_value = nullptr;

	for (auto n = camera_value.MemberBegin(); n != camera_value.MemberEnd(); ++n) {
		type_name = n->name.GetString();
		type_value = &n->value;
	}

	math::float3 position = math::float3::identity;
	math::quaternion rotation = math::quaternion::identity;
	math::float2 dimensions = math::float2::identity;
	rendering::film::Film* film = nullptr;
	float fov = 60.f;

	for (auto n = type_value->MemberBegin(); n != type_value->MemberEnd(); ++n) {
		const std::string node_name = n->name.GetString();
		const rapidjson::Value& node_value = n->value;

		if ("position" == node_name) {
			position = json::read_float3(node_value);
		} else if ("rotation" == node_name) {
			rotation = json::read_local_rotation(node_value);
		} else if ("dimensions" == node_name) {
			dimensions = json::read_float2(node_value);
		} else if ("film" == node_name) {
			film = load_film(node_value);
		} else if ("fov" == node_name) {
			fov = math::degrees_to_radians(json::read_float(node_value));
		}
	}

	std::shared_ptr<camera::Camera> camera;

	if ("Perspective" == type_name) {
		camera = std::make_shared<camera::Perspective>(dimensions, film, fov);
	} else if ("Orthographic" == type_name) {

	}

	camera->set_transformation(position, math::float3(1.f, 1.f, 1.f), rotation);

	camera->update_view();

	return camera;
}

rendering::film::Film* Loader::load_film(const rapidjson::Value& film_value) const {
	math::uint2 dimensions(32, 32);

	for (auto n = film_value.MemberBegin(); n != film_value.MemberEnd(); ++n) {
		const std::string node_name = n->name.GetString();
		const rapidjson::Value& node_value = n->value;

		if ("dimensions" == node_name) {
			dimensions = json::read_uint2(node_value);
		}
	}

	rendering::film::Film* film = new rendering::film::Unfiltered(dimensions);

	return film;
}

std::shared_ptr<rendering::Surface_integrator_factory> Loader::load_surface_integrator_factory(const rapidjson::Value& integrator_value) const {
	return std::make_shared<rendering::Ao_factory>(16, 5.f);
}

}
