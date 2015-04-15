#include "take_loader.hpp"
#include "take.hpp"
#include "rendering/film/filtered.hpp"
#include "rendering/film/unfiltered.hpp"
#include "rendering/film/filter/gaussian.hpp"
#include "rendering/film/tonemapping/filmic.hpp"
#include "rendering/film/tonemapping/identity.hpp"
#include "rendering/integrator/surface/ao.hpp"
#include "rendering/integrator/surface/whitted.hpp"
#include "rendering/integrator/surface/pathtracer_dl.hpp"
#include "sampler/random_sampler.hpp"
#include "sampler/scrambled_hammersley_sampler.hpp"
#include "sampler/ems_sampler.hpp"
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
		throw std::runtime_error("Could not open file");
	}

	auto root = json::parse(stream);

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

	if (take->scene.empty()) {
		throw std::runtime_error("No reference to scene included");
	}

	if (!take->context.camera) {
		throw std::runtime_error("No camera configuration included");
	}

	if (!take->sampler) {
		take->sampler = std::make_shared<sampler::Random>(1, take->rng);
	}

	if (!take->surface_integrator_factory) {
		take->surface_integrator_factory = std::make_shared<rendering::Pathtracer_DL_factory>(4, 4);
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
	float lens_radius = 0.f;
	float focal_distance = 0.f;

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
		} else if ("lens_radius" == node_name) {
			lens_radius = json::read_float(node_value);
		} else if ("focal_distance" == node_name) {
			focal_distance = json::read_float(node_value);
		}
	}

	std::shared_ptr<scene::camera::Camera> camera;

//	if ("Perspective" == type_name) {
		camera = std::make_shared<scene::camera::Perspective>(dimensions, film, fov, lens_radius, focal_distance);
//	} else if ("Orthographic" == type_name) {
//	}

	camera->set_transformation(position, math::float3(1.f, 1.f, 1.f), rotation);

	camera->update_view();

	return camera;
}

rendering::film::Film* Loader::load_film(const rapidjson::Value& film_value) const {
	math::uint2 dimensions(32, 32);
	float exposure = 0.f;
	rendering::film::filter::Filter* filter = nullptr;
	rendering::film::tonemapping::Tonemapper* tonemapper = nullptr;

	for (auto n = film_value.MemberBegin(); n != film_value.MemberEnd(); ++n) {
		const std::string node_name = n->name.GetString();
		const rapidjson::Value& node_value = n->value;

		if ("dimensions" == node_name) {
			dimensions = json::read_uint2(node_value);
		} else if ("exposure" == node_name) {
			exposure = json::read_float(node_value);
		} else if ("tonemapper" == node_name) {
			tonemapper = load_tonemapper(node_value);
		} else if ("filter" == node_name) {
			filter = load_filter(node_value);
		}
	}

	if (!tonemapper) {
		tonemapper = new rendering::film::tonemapping::Identity;
	}

	if (filter) {
		return new rendering::film::Filtered(dimensions, exposure, tonemapper, filter);
	}

	return new rendering::film::Unfiltered(dimensions, exposure, tonemapper);
}

rendering::film::tonemapping::Tonemapper* Loader::load_tonemapper(const rapidjson::Value& tonemapper_value) const {
	for (auto n = tonemapper_value.MemberBegin(); n != tonemapper_value.MemberEnd(); ++n) {
		const std::string type_name = n->name.GetString();
		const rapidjson::Value& type_value = n->value;

		if ("Filmic" == type_name) {
			math::float3 linear_white = json::read_float3(type_value, "linear_white");
			return new rendering::film::tonemapping::Filmic(linear_white);
		} else if ("Identity" == type_name) {
			return new rendering::film::tonemapping::Identity;
		}
	}

	return nullptr;
}

rendering::film::filter::Filter* Loader::load_filter(const rapidjson::Value& film_value) const {
	float radius = 0.8f;
	float alpha  = 0.3f;
	return new rendering::film::filter::Gaussian(math::float2(radius, radius), alpha);
}

std::shared_ptr<sampler::Sampler> Loader::load_sampler(const rapidjson::Value& sampler_value, math::random::Generator& rng) const {
	for (auto n = sampler_value.MemberBegin(); n != sampler_value.MemberEnd(); ++n) {
		const std::string type_name = n->name.GetString();
		const rapidjson::Value& type_value = n->value;

		 if ("Random" == type_name) {
			uint32_t num_samples = json::read_uint(type_value, "samples_per_pixel");
			return std::make_shared<sampler::Random>(num_samples, rng);
		} else if ("Scrambled_hammersley" == type_name) {
			 uint32_t num_samples = json::read_uint(type_value, "samples_per_pixel");
			 return std::make_shared<sampler::Scrambled_hammersley>(num_samples, rng);
		 } else if ("EMS" == type_name) {
			 uint32_t num_samples = json::read_uint(type_value, "samples_per_pixel");
			 return std::make_shared<sampler::EMS>(num_samples, rng);
		 }
	}

	return nullptr;
}

std::shared_ptr<rendering::Surface_integrator_factory> Loader::load_surface_integrator_factory(const rapidjson::Value& integrator_value) const {
	for (auto n = integrator_value.MemberBegin(); n != integrator_value.MemberEnd(); ++n) {
		const std::string type_name = n->name.GetString();
		const rapidjson::Value& type_value = n->value;

		if ("AO" == type_name) {
			uint32_t num_samples = json::read_uint(type_value, "num_samples", 1);
			float radius = json::read_float(type_value, "radius", 1.f);
			return std::make_shared<rendering::Ao_factory>(num_samples, radius);
		} else if ("Whitted" == type_name) {
			return std::make_shared<rendering::Whitted_factory>();
		} else if ("PTDL" == type_name) {
			uint32_t min_bounces = json::read_uint(type_value, "min_bounces", 4);
			uint32_t max_bounces = json::read_uint(type_value, "max_bounces", 4);
			return std::make_shared<rendering::Pathtracer_DL_factory>(min_bounces, max_bounces);
		}
	}

	return nullptr;
}

}
