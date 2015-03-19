#include "take_loader.hpp"
#include "take.hpp"
#include "rendering/film/filtered.hpp"
#include "rendering/film/filter/gaussian.hpp"
#include "rendering/film/unfiltered.hpp"
#include "rendering/integrator/surface/ao.hpp"
#include "rendering/sampler/scrambled_hammersley_sampler.hpp"
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
			take->sampler = load_sampler(node_value, take->rng);
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

std::shared_ptr<scene::camera::Camera> Loader::load_camera(const rapidjson::Value& camera_value) const {
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

	std::shared_ptr<scene::camera::Camera> camera;

	if ("Perspective" == type_name) {
		camera = std::make_shared<scene::camera::Perspective>(dimensions, film, fov);
	} else if ("Orthographic" == type_name) {

	}

	camera->set_transformation(position, math::float3(1.f, 1.f, 1.f), rotation);

	camera->update_view();

	return camera;
}

rendering::film::Film* Loader::load_film(const rapidjson::Value& film_value) const {
	math::uint2 dimensions(32, 32);

	rendering::film::filter::Filter* filter = nullptr;

	for (auto n = film_value.MemberBegin(); n != film_value.MemberEnd(); ++n) {
		const std::string node_name = n->name.GetString();
		const rapidjson::Value& node_value = n->value;

		if ("dimensions" == node_name) {
			dimensions = json::read_uint2(node_value);
		} else if ("filter" == node_name) {
			filter = load_filter(node_value);
		}
	}

//	float radius = 1.5f;
//	rendering::film::filter::Filter* filter = new rendering::film::filter::Gaussian(math::float2(radius, radius), 50.0f);

	if (filter) {
		return new rendering::film::Filtered(dimensions, filter);
	}

	return new rendering::film::Unfiltered(dimensions);
}

rendering::film::filter::Filter* Loader::load_filter(const rapidjson::Value& film_value) const {
	float radius = 1.f;
	float alpha = 0.3f;
	return new rendering::film::filter::Gaussian(math::float2(radius, radius), alpha);
}

std::shared_ptr<rendering::sampler::Sampler> Loader::load_sampler(const rapidjson::Value& sampler_value, math::random::Generator& rng) const {
	for (auto n = sampler_value.MemberBegin(); n != sampler_value.MemberEnd(); ++n) {
		const std::string type_name = n->name.GetString();
		const rapidjson::Value& type_value = n->value;

		if ("Scrambled_hammersley" == type_name) {
			uint32_t num_samples = json::read_uint(type_value, "samples_per_pixel");
			return std::make_shared<rendering::sampler::Scrambled_hammersley>(num_samples, rng);
		} else if ("Random" == type_name) {
			uint32_t num_samples = json::read_uint(type_value, "samples_per_pixel");
			return std::make_shared<rendering::sampler::Random>(num_samples, rng);
		}
	}

	return nullptr;
}

std::shared_ptr<rendering::Surface_integrator_factory> Loader::load_surface_integrator_factory(const rapidjson::Value& integrator_value) const {
	return std::make_shared<rendering::Ao_factory>(16, 5.f);
}

}
