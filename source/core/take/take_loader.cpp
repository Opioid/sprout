#include "take_loader.hpp"
#include "take.hpp"
#include "exporting/exporting_sink_ffmpeg.hpp"
#include "exporting/exporting_sink_image_sequence.hpp"
#include "exporting/exporting_sink_null.hpp"
#include "image/encoding/png/png_writer.hpp"
#include "image/encoding/rgbe/rgbe_writer.hpp"
#include "rendering/sensor/opaque.hpp"
#include "rendering/sensor/transparent.hpp"
#include "rendering/sensor/clamp.inl"
#include "rendering/sensor/filtered.inl"
#include "rendering/sensor/unfiltered.inl"
#include "rendering/sensor/filter/gaussian.inl"
#include "rendering/sensor/tonemapping/filmic.hpp"
#include "rendering/sensor/tonemapping/identity.hpp"
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
#include "scene/camera/camera_cubic.hpp"
#include "scene/camera/camera_cubic_stereoscopic.hpp"
#include "scene/camera/camera_perspective.hpp"
#include "scene/camera/camera_perspective_stereoscopic.hpp"
#include "scene/camera/camera_spherical.hpp"
#include "scene/camera/camera_spherical_stereoscopic.hpp"
#include "base/math/math.hpp"
#include "base/math/vector.inl"
#include "base/math/matrix.inl"
#include "base/math/quaternion.inl"
#include "base/json/json.hpp"

namespace take {

void load_focus(const rapidjson::Value& focus_value, scene::camera::Perspective::Focus& focus);

std::shared_ptr<Take> Loader::load(std::istream& stream) {
	auto root = json::parse(stream);

	auto take = std::make_shared<Take>();

	const rapidjson::Value* exporter_value = nullptr;
	bool alpha_transparency = peek_alpha_transparency(*root);

	for (auto n = root->MemberBegin(); n != root->MemberEnd(); ++n) {
		const std::string node_name = n->name.GetString();
		const rapidjson::Value& node_value = n->value;

		if ("camera" == node_name) {
			load_camera(node_value, alpha_transparency, *take);
		} else if ("export" == node_name) {
			exporter_value = &node_value;
		} else if ("start_frame" == node_name) {
			take->context.start_frame = json::read_uint(node_value);
		} else if ("num_frames" == node_name) {
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
		image::Writer* writer = new image::encoding::png::Writer(take->context.camera->sensor().dimensions());
		take->exporter = std::make_unique<exporting::Image_sequence>("output_", writer);
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

void Loader::load_camera(const rapidjson::Value& camera_value, bool alpha_transparency, Take& take) const {
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
	const rapidjson::Value* sensor_value = nullptr;
	std::string layout_type;
	Stereoscopic stereo;
	float frame_duration = 0.f;
	bool  motion_blur = true;
	scene::camera::Perspective::Focus focus;
	float fov = math::degrees_to_radians(60.f);
	float lens_radius = 0.f;

	for (auto n = type_value->MemberBegin(); n != type_value->MemberEnd(); ++n) {
		const std::string node_name = n->name.GetString();
		const rapidjson::Value& node_value = n->value;

		if ("transformation" == node_name) {
			json::read_transformation(node_value, transformation);
		} else if ("animation" == node_name) {
			animation_value = &node_value;
		} else if ("sensor" == node_name) {
			sensor_value = &node_value;
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
		} else if ("focus" == node_name) {
			load_focus(node_value, focus);
		} else if ("fov" == node_name) {
			fov = math::degrees_to_radians(json::read_float(node_value));
		} else if ("lens_radius" == node_name) {
			lens_radius = json::read_float(node_value);
		} else if ("layout" == node_name) {
			layout_type = json::read_string(node_value);
		} else if ("stereoscopic" == node_name) {
			load_stereoscopic(node_value, stereo);
		}
	}

	math::int2 resolution;
	if (sensor_value) {
		resolution = json::read_int2(*sensor_value, "resolution");
		if (math::int2::identity == resolution) {
			throw std::runtime_error("Sensor resolution is [0, 0]");
		}
	} else {
		throw std::runtime_error("No sensor configuration included");
	}

	if (animation_value) {
		take.camera_animation = scene::animation::load(*animation_value, transformation);
	}

	std::shared_ptr<scene::camera::Camera> camera;

	if ("Cubic" == type_name) {
		if (stereo.interpupillary_distance > 0.f) {
			scene::camera::Cubic_stereoscopic::Layout layout =
					scene::camera::Cubic_stereoscopic::Layout::rxlmxryrmyrzrmzlxlmxlylmylzlmz;

			if ("lxlmxlylmylzlmzrxrmxryrmyrzrmz" == layout_type) {
				layout = scene::camera::Cubic_stereoscopic::Layout::lxlmxlylmylzlmzrxrmxryrmyrzrmz;
			}

			camera = std::make_shared<scene::camera::Cubic_stereoscopic>(
						layout, stereo.interpupillary_distance, resolution,
						take.settings.ray_max_t, frame_duration, motion_blur);
		} else {
			scene::camera::Cubic::Layout layout = scene::camera::Cubic::Layout::xmxymyzmz;

			if ("xmxy_myzmz" == layout_type) {
				layout = scene::camera::Cubic::Layout::xmxy_myzmz;
			}

			camera = std::make_shared<scene::camera::Cubic>(
						layout, resolution, take.settings.ray_max_t, frame_duration, motion_blur);
		}
	} else if ("Perspective" == type_name) {
		if (stereo.interpupillary_distance > 0.f) {
			camera = std::make_shared<scene::camera::Perspective_stereoscopic>(
						stereo.interpupillary_distance, resolution,
						take.settings.ray_max_t, frame_duration, motion_blur, fov);
		} else {
			camera = std::make_shared<scene::camera::Perspective>(
						resolution, take.settings.ray_max_t, frame_duration, motion_blur, focus, fov, lens_radius);
		}
	} else if ("Spherical" == type_name) {
		if (stereo.interpupillary_distance > 0.f) {
			camera = std::make_shared<scene::camera::Spherical_stereoscopic>(
						stereo.interpupillary_distance, resolution,
						take.settings.ray_max_t, frame_duration, motion_blur);
		} else {
			camera = std::make_shared<scene::camera::Spherical>(
						resolution, take.settings.ray_max_t, frame_duration, motion_blur);
		}
	} else {
		throw std::runtime_error("Camera type \"" + type_name + "\" not recognized");
	}

	rendering::sensor::Sensor* sensor = load_sensor(*sensor_value, camera->sensor_dimensions(), alpha_transparency);

	camera->set_sensor(sensor);
	camera->set_transformation(transformation);

	take.context.camera = camera;
}

void Loader::load_stereoscopic(const rapidjson::Value& stereo_value, Stereoscopic& stereo) const {
	for (auto n = stereo_value.MemberBegin(); n != stereo_value.MemberEnd(); ++n) {
		const std::string node_name = n->name.GetString();
		const rapidjson::Value& node_value = n->value;

		if ("interpupillary_distance" == node_name) {
			stereo.interpupillary_distance = json::read_float(node_value);
		}
	}
}

rendering::sensor::Sensor* Loader::load_sensor(const rapidjson::Value& sensor_value,
											   math::int2 dimensions, bool alpha_transparency) const {
	float exposure = 0.f;
	math::float3 clamp_max(-1.f, -1.f, -1.f);
	std::unique_ptr<rendering::sensor::tonemapping::Tonemapper> tonemapper;
	bool filter = false;

	for (auto n = sensor_value.MemberBegin(); n != sensor_value.MemberEnd(); ++n) {
		const std::string node_name = n->name.GetString();
		const rapidjson::Value& node_value = n->value;

		if ("exposure" == node_name) {
			exposure = json::read_float(node_value);
		} else if ("clamp" == node_name) {
			clamp_max = json::read_float3(node_value);
		} else if ("tonemapper" == node_name) {
			tonemapper = load_tonemapper(node_value);
		} else if ("filter" == node_name) {
			//filter = load_filter(node_value);
			filter = true;
		}
	}

	if (!tonemapper) {
		tonemapper = std::make_unique<rendering::sensor::tonemapping::Identity>();
	}

	bool clamp = !math::contains_negative(clamp_max);

	if (filter) {
		float radius = 0.8f;
		float alpha  = 0.3f;
		auto gaussian = std::make_unique<rendering::sensor::filter::Gaussian>(radius, alpha);

		if (alpha_transparency) {
			if (clamp) {
				return new rendering::sensor::Filtered<
						rendering::sensor::Transparent,
						rendering::sensor::clamp::Clamp,
						rendering::sensor::filter::Gaussian>(
							dimensions, exposure, std::move(tonemapper),
							rendering::sensor::clamp::Clamp(clamp_max), std::move(gaussian));
			} else {
				return new rendering::sensor::Filtered<
						rendering::sensor::Transparent,
						rendering::sensor::clamp::Identity,
						rendering::sensor::filter::Gaussian>(
							dimensions, exposure, std::move(tonemapper),
							rendering::sensor::clamp::Identity(), std::move(gaussian));
			}
		}

		if (clamp) {
			return new rendering::sensor::Filtered<
					rendering::sensor::Opaque,
					rendering::sensor::clamp::Clamp,
					rendering::sensor::filter::Gaussian>(
						dimensions, exposure, std::move(tonemapper),
						rendering::sensor::clamp::Clamp(clamp_max), std::move(gaussian));
		} else {
			return new rendering::sensor::Filtered<
					rendering::sensor::Opaque,
					rendering::sensor::clamp::Identity,
					rendering::sensor::filter::Gaussian>(
						dimensions, exposure, std::move(tonemapper),
						rendering::sensor::clamp::Identity(), std::move(gaussian));
		}
	}

	if (alpha_transparency) {
		if (clamp) {
			return new rendering::sensor::Unfiltered<
					rendering::sensor::Transparent,
					rendering::sensor::clamp::Clamp>
					(dimensions, exposure, std::move(tonemapper), rendering::sensor::clamp::Clamp(clamp_max));
		} else {
			return new rendering::sensor::Unfiltered<
					rendering::sensor::Transparent,
					rendering::sensor::clamp::Identity>
					(dimensions, exposure, std::move(tonemapper), rendering::sensor::clamp::Identity());
		}
	}

	if (clamp) {
		return new rendering::sensor::Unfiltered<
				rendering::sensor::Opaque,
				rendering::sensor::clamp::Clamp>
				(dimensions, exposure, std::move(tonemapper), rendering::sensor::clamp::Clamp(clamp_max));
	}

	return new rendering::sensor::Unfiltered<
			rendering::sensor::Opaque,
			rendering::sensor::clamp::Identity>
			(dimensions, exposure, std::move(tonemapper), rendering::sensor::clamp::Identity());
}

std::unique_ptr<rendering::sensor::tonemapping::Tonemapper>
Loader::load_tonemapper(const rapidjson::Value& tonemapper_value) const {
	for (auto n = tonemapper_value.MemberBegin(); n != tonemapper_value.MemberEnd(); ++n) {
		const std::string node_name = n->name.GetString();
		const rapidjson::Value& node_value = n->value;

		if ("Filmic" == node_name) {
			math::float3 linear_white = json::read_float3(node_value, "linear_white");
			return std::make_unique<rendering::sensor::tonemapping::Filmic>(linear_white);
		} else if ("Identity" == node_name) {
			return std::make_unique<rendering::sensor::tonemapping::Identity>();
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
			uint32_t num_light_samples = json::read_uint(node_value, "num_light_samples", default_max_light_samples);
			return std::make_shared<rendering::Whitted_factory>(settings, num_light_samples);
		} else if ("PT" == node_name) {
			uint32_t min_bounces = json::read_uint(node_value, "min_bounces", default_min_bounces);
			uint32_t max_bounces = json::read_uint(node_value, "max_bounces", default_max_bounces);
			bool disable_caustics = !json::read_bool(node_value, "caustics", true);
			return std::make_shared<rendering::Pathtracer_factory>(settings, min_bounces, max_bounces,
																   disable_caustics);
		} else if ("PTDL" == node_name) {
			uint32_t min_bounces = json::read_uint(node_value, "min_bounces", default_min_bounces);
			uint32_t max_bounces = json::read_uint(node_value, "max_bounces", default_max_bounces);
			uint32_t num_light_samples = json::read_uint(node_value, "num_light_samples", default_max_light_samples);
			bool disable_caustics = !json::read_bool(node_value, "caustics", true);
			return std::make_shared<rendering::Pathtracer_DL_factory>(settings, min_bounces, max_bounces,
																	  num_light_samples, disable_caustics);
		} else if ("PTMIS" == node_name) {
			uint32_t min_bounces = json::read_uint(node_value, "min_bounces", default_min_bounces);
			uint32_t max_bounces = json::read_uint(node_value, "max_bounces", default_max_bounces);
			uint32_t num_light_samples = json::read_uint(node_value, "num_light_samples", default_max_light_samples);
			bool disable_caustics = !json::read_bool(node_value, "caustics", true);
			return std::make_shared<rendering::Pathtracer_MIS_factory>(settings, min_bounces, max_bounces,
																	   num_light_samples, disable_caustics);
		} else if ("Normal" == node_name) {
			rendering::Normal::Settings::Vector vector = rendering::Normal::Settings::Vector::Shading_normal;

			std::string vector_type = json::read_string(node_value, "vector");

			if ("Tangent" == vector_type) {
				vector = rendering::Normal::Settings::Vector::Tangent;
			} else if ("Bitangent" == vector_type) {
				vector = rendering::Normal::Settings::Vector::Bitangent;
			} else if ("Geometric_normal" == vector_type) {
				vector = rendering::Normal::Settings::Vector::Geometric_normal;
			} else if ("Shading_normal" == vector_type) {
				vector = rendering::Normal::Settings::Vector::Shading_normal;
			}

			return std::make_shared<rendering::Normal_factory>(settings, vector);
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
			const std::string format = json::read_string(node_value, "format", "PNG");

			image::Writer* writer;

			if ("RGBE" == format) {
				writer = new image::encoding::rgbe::Writer;
			} else {
				writer = new image::encoding::png::Writer(camera.sensor().dimensions());
			}

			return std::make_unique<exporting::Image_sequence>("output_", writer);
		} else if ("Movie" == node_name) {
			uint32_t framerate = json::read_uint(node_value, "framerate");

			if (!framerate) {
				framerate = static_cast<uint32_t>(1.f / camera.frame_duration() + 0.5f);
			}

			return std::make_unique<exporting::Ffmpeg>("output", camera.sensor().dimensions(), framerate);
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

		if ("ray_offset_factor" == node_name) {
			settings.ray_offset_factor = json::read_float(node_value);
		}
	}
}

void load_focus(const rapidjson::Value& focus_value, scene::camera::Perspective::Focus& focus) {
	for (auto n = focus_value.MemberBegin(); n != focus_value.MemberEnd(); ++n) {
		const std::string node_name = n->name.GetString();
		const rapidjson::Value& node_value = n->value;

		if ("point" == node_name) {
			focus.point = json::read_float3(node_value);
			focus.use_point = true;
		} else if ("distance" == node_name) {
			focus.distance = json::read_float(node_value);
		}
	}
}

}
