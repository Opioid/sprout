#include "take_loader.hpp"
#include "take.hpp"
#include "exporting/exporting_sink_ffmpeg.hpp"
#include "exporting/exporting_sink_image_sequence.hpp"
#include "exporting/exporting_sink_null.hpp"
#include "rendering/film/opaque.hpp"
#include "rendering/film/transparent.hpp"
#include "rendering/film/filtered.inl"
#include "rendering/film/unfiltered.inl"
#include "rendering/film/filter/gaussian.inl"
#include "rendering/film/tonemapping/filmic.hpp"
#include "rendering/film/tonemapping/identity.hpp"
#include "rendering/integrator/surface/ao.hpp"
#include "rendering/integrator/surface/normal.hpp"
#include "rendering/integrator/surface/whitted.hpp"
#include "rendering/integrator/surface/pathtracer.hpp"
#include "rendering/integrator/surface/pathtracer_dl.hpp"
#include "rendering/integrator/surface/pathtracer_mis.hpp"
#include "sampler/ems_sampler.hpp"
#include "sampler/ld_sampler.hpp"
#include "sampler/random_sampler.hpp"
#include "sampler/scrambled_hammersley_sampler.hpp"
#include "scene/animation/animation_loader.hpp"
#include "scene/camera/perspective_camera.hpp"
#include "base/math/math.hpp"
#include "base/math/vector.inl"
#include "base/math/matrix.inl"
#include "base/math/quaternion.inl"
#include "base/json/json.hpp"
#include <iostream>

namespace take {

std::shared_ptr<Take> Loader::load(std::istream& stream, thread::Pool& pool) {
	auto root = json::parse(stream);

	auto take = std::make_shared<Take>();

	const rapidjson::Value* exporter_value = nullptr;
	bool alpha_transparency = peek_alpha_transparency(*root);

	for (auto n = root->MemberBegin(); n != root->MemberEnd(); ++n) {
		const std::string node_name = n->name.GetString();
		const rapidjson::Value& node_value = n->value;

		if ("camera" == node_name) {
			load_camera(node_value, alpha_transparency, pool, *take);
		} else if ("export" == node_name) {
			exporter_value = &node_value;
		} else if ("frames" == node_name) {
			take->context.num_frames = json::read_uint(node_value);
		} else if ("integrator" == node_name) {
			take->surface_integrator_factory = load_surface_integrator_factory(node_value, take->settings);
		} else if ("sampler" == node_name) {
			take->sampler = load_sampler(node_value, take->rng);
		} else if ("scene" == node_name) {
			take->scene = node_value.GetString();
		} else if ("settings" == node_name) {
			load_settings(node_value, take->settings);
		}
	}

	if (take->scene.empty()) {
		throw std::runtime_error("No reference to scene included");
	}

	if (!take->context.camera) {
		throw std::runtime_error("No camera configuration included");
	}

	if (exporter_value) {
		take->exporter = load_exporter(*exporter_value, *take->context.camera);
	}

	if (!take->exporter) {
		take->exporter = std::make_unique<exporting::Image_sequence>(
							"output_", take->context.camera->film().dimensions());
	}

	if (!take->sampler) {
		take->sampler = std::make_shared<sampler::Random>(take->rng, 1);
	}

	if (!take->surface_integrator_factory) {
		take->surface_integrator_factory = std::make_shared<rendering::Pathtracer_DL_factory>(
					take->settings, 4, 8, 1, false);
	}

	return take;
}

void Loader::load_camera(const rapidjson::Value& camera_value, bool alpha_transparency, thread::Pool& pool,
						 Take& take) const {
	std::string type_name = "Perspective";
	const rapidjson::Value* type_value = nullptr;

	for (auto n = camera_value.MemberBegin(); n != camera_value.MemberEnd(); ++n) {
		type_name = n->name.GetString();
		type_value = &n->value;
	}

	math::transformation transformation{
		math::float3::identity,
		math::float3(1.f, 1.f, 1.f),
		math::quaternion::identity
	};

	const rapidjson::Value* animation_value = nullptr;
	math::float2 dimensions = math::float2::identity;
	rendering::film::Film* film = nullptr;
	float frame_duration = 0.f;
	bool  motion_blur = true;
	float fov = 60.f;
	float lens_radius = 0.f;
	float focal_distance = 0.f;

	for (auto n = type_value->MemberBegin(); n != type_value->MemberEnd(); ++n) {
		const std::string node_name = n->name.GetString();
		const rapidjson::Value& node_value = n->value;

		if ("transformation" == node_name) {
			json::read_transformation(node_value, transformation);
		} else if ("animation" == node_name) {
			animation_value = &node_value;
		} else if ("dimensions" == node_name) {
			dimensions = json::read_float2(node_value);
		} else if ("film" == node_name) {
			film = load_film(node_value, alpha_transparency);
		} else if ("frame_duration" == node_name) {
			frame_duration = json::read_float(node_value);
		} else if ("frames_per_second" == node_name) {
			float fps = json::read_float(node_value);
			if (0.f == fps) {
				frame_duration = 0.f;
			} else {
				frame_duration = 1.f / fps;
			}
		} else if ("motion_blur" == node_name) {
			motion_blur = json::read_bool(node_value);
		} else if ("fov" == node_name) {
			fov = math::degrees_to_radians(json::read_float(node_value));
		} else if ("lens_radius" == node_name) {
			lens_radius = json::read_float(node_value);
		} else if ("focal_distance" == node_name) {
			focal_distance = json::read_float(node_value);
		}
	}

	if (animation_value) {
		take.camera_animation = scene::animation::load(*animation_value, transformation);
	}

	std::shared_ptr<scene::camera::Camera> camera;

//	if ("Perspective" == type_name) {
		camera = std::make_shared<scene::camera::Perspective>(dimensions, film, frame_duration, motion_blur,
															  fov, lens_radius, focal_distance);
//	} else if ("Orthographic" == type_name) {
//	}

	camera->set_transformation(transformation, pool);

	camera->update_view();

	take.context.camera = camera;
}

rendering::film::Film* Loader::load_film(const rapidjson::Value& film_value, bool alpha_transparency) const {
	math::uint2 dimensions(1280, 720);
	float exposure = 0.f;
	rendering::film::tonemapping::Tonemapper* tonemapper = nullptr;
	bool filter = false;

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
			//filter = load_filter(node_value);
			filter = true;
		}
	}

	if (!tonemapper) {
		tonemapper = new rendering::film::tonemapping::Identity;
	}

	if (filter) {
		float radius = 0.8f;
		float alpha  = 0.3f;
		auto gaussian = std::make_unique<rendering::film::filter::Gaussian>(radius, alpha);

		if (alpha_transparency) {
			return new rendering::film::Filtered<rendering::film::Transparent, rendering::film::filter::Gaussian>(
						dimensions, exposure, tonemapper, std::move(gaussian));
		}

		return new rendering::film::Filtered<rendering::film::Opaque, rendering::film::filter::Gaussian>(
					dimensions, exposure, tonemapper, std::move(gaussian));
	}

	if (alpha_transparency) {
		return new rendering::film::Unfiltered<rendering::film::Transparent>(dimensions, exposure, tonemapper);
	}

	return new rendering::film::Unfiltered<rendering::film::Opaque>(dimensions, exposure, tonemapper);
}

rendering::film::tonemapping::Tonemapper* Loader::load_tonemapper(const rapidjson::Value& tonemapper_value) const {
	for (auto n = tonemapper_value.MemberBegin(); n != tonemapper_value.MemberEnd(); ++n) {
		const std::string node_name = n->name.GetString();
		const rapidjson::Value& node_value = n->value;

		if ("Filmic" == node_name) {
			math::float3 linear_white = json::read_float3(node_value, "linear_white");
			return new rendering::film::tonemapping::Filmic(linear_white);
		} else if ("Identity" == node_name) {
			return new rendering::film::tonemapping::Identity;
		}
	}

	return nullptr;
}

std::shared_ptr<sampler::Sampler> Loader::load_sampler(const rapidjson::Value& sampler_value,
													   math::random::Generator& rng) const {
	for (auto n = sampler_value.MemberBegin(); n != sampler_value.MemberEnd(); ++n) {
		const std::string node_name = n->name.GetString();
		const rapidjson::Value& node_value = n->value;

		 if ("Random" == node_name) {
			uint32_t num_samples = json::read_uint(node_value, "samples_per_pixel");
			return std::make_shared<sampler::Random>(rng, num_samples);
		} else if ("Scrambled_hammersley" == node_name) {
			 uint32_t num_samples = json::read_uint(node_value, "samples_per_pixel");
			 return std::make_shared<sampler::Scrambled_hammersley>(rng, num_samples);
		 } else if ("EMS" == node_name) {
			 uint32_t num_samples = json::read_uint(node_value, "samples_per_pixel");
			 return std::make_shared<sampler::EMS>(rng, num_samples);
		 } else if ("LD" == node_name) {
			 uint32_t num_samples = json::read_uint(node_value, "samples_per_pixel");
			 return std::make_shared<sampler::LD>(rng, num_samples);
		 }
	}

	return nullptr;
}

std::shared_ptr<rendering::Surface_integrator_factory>
Loader::load_surface_integrator_factory(const rapidjson::Value& integrator_value,
										const Settings& settings) const {
	uint32_t default_min_bounces = 4;
	uint32_t default_max_bounces = 8;
	uint32_t default_max_light_samples = 1;

	for (auto n = integrator_value.MemberBegin(); n != integrator_value.MemberEnd(); ++n) {
		const std::string node_name = n->name.GetString();
		const rapidjson::Value& node_value = n->value;

		if ("AO" == node_name) {
			uint32_t num_samples = json::read_uint(node_value, "num_samples", 1);
			float radius = json::read_float(node_value, "radius", 1.f);
			return std::make_shared<rendering::Ao_factory>(settings, num_samples, radius);
		} else if ("Whitted" == node_name) {
			uint32_t max_light_samples = json::read_uint(node_value, "max_light_samples", default_max_light_samples);
			return std::make_shared<rendering::Whitted_factory>(settings, max_light_samples);
		} else if ("PT" == node_name) {
			uint32_t min_bounces = json::read_uint(node_value, "min_bounces", default_min_bounces);
			uint32_t max_bounces = json::read_uint(node_value, "max_bounces", default_max_bounces);
			bool disable_caustics = !json::read_bool(node_value, "caustics", true);
			return std::make_shared<rendering::Pathtracer_factory>(settings, min_bounces, max_bounces,
																   disable_caustics);
		} else if ("PTDL" == node_name) {
			uint32_t min_bounces = json::read_uint(node_value, "min_bounces", default_min_bounces);
			uint32_t max_bounces = json::read_uint(node_value, "max_bounces", default_max_bounces);
			uint32_t max_light_samples = json::read_uint(node_value, "max_light_samples", default_max_light_samples);
			bool disable_caustics = !json::read_bool(node_value, "caustics", true);
			return std::make_shared<rendering::Pathtracer_DL_factory>(settings, min_bounces, max_bounces,
																	  max_light_samples, disable_caustics);
		} else if ("PTMIS" == node_name) {
			uint32_t min_bounces = json::read_uint(node_value, "min_bounces", default_min_bounces);
			uint32_t max_bounces = json::read_uint(node_value, "max_bounces", default_max_bounces);
			uint32_t max_light_samples = json::read_uint(node_value, "max_light_samples", default_max_light_samples);
			bool disable_caustics = !json::read_bool(node_value, "caustics", true);
			return std::make_shared<rendering::Pathtracer_MIS_factory>(settings, min_bounces, max_bounces,
																	   max_light_samples, disable_caustics);
		} else if ("Normal" == node_name) {
			return std::make_shared<rendering::Normal_factory>(settings);
		}
	}

	return nullptr;
}

bool Loader::peek_alpha_transparency(const rapidjson::Value& take_value) const {
	const rapidjson::Value::ConstMemberIterator export_node = take_value.FindMember("export");
	if (take_value.MemberEnd() == export_node) {
		return false;
	}

	const rapidjson::Value::ConstMemberIterator node = export_node->value.FindMember("Image");
	if (export_node->value.MemberEnd() == node) {
		return false;
	}

	return json::read_bool(node->value, "alpha_transparency");
}

std::unique_ptr<exporting::Sink> Loader::load_exporter(const rapidjson::Value& exporter_value,
													   scene::camera::Camera& camera) const {
	for (auto n = exporter_value.MemberBegin(); n != exporter_value.MemberEnd(); ++n) {
		const std::string node_name = n->name.GetString();
		const rapidjson::Value& node_value = n->value;

		if ("Image" == node_name) {
			return std::make_unique<exporting::Image_sequence>("output_", camera.film().dimensions());
		} else if ("Movie" == node_name) {
			uint32_t framerate = json::read_uint(node_value, "framerate");

			if (!framerate) {
				framerate = static_cast<uint32_t>(1.f / camera.frame_duration() + 0.5f);
			}

			return std::make_unique<exporting::Ffmpeg>("output", camera.film().dimensions(), framerate);
		} else if ("Null" == node_name) {
			return std::make_unique<exporting::Null>();
		}
	}

	return nullptr;
}

void Loader::load_settings(const rapidjson::Value& settings_value, Settings& settings) const {
	for (auto n = settings_value.MemberBegin(); n != settings_value.MemberEnd(); ++n) {
		const std::string node_name = n->name.GetString();
		const rapidjson::Value& node_value = n->value;

		if ("ray_offset_modifier" == node_name) {
			settings.ray_offset_modifier = json::read_float(node_value);
		}
	}
}

}
