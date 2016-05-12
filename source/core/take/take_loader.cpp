#include "take_loader.hpp"
#include "take.hpp"
#include "exporting/exporting_sink_ffmpeg.hpp"
#include "exporting/exporting_sink_image_sequence.hpp"
#include "exporting/exporting_sink_null.hpp"
#include "exporting/exporting_sink_statistics.hpp"
#include "image/encoding/png/png_writer.hpp"
#include "image/encoding/rgbe/rgbe_writer.hpp"
#include "logging/logging.hpp"
#include "rendering/sensor/opaque.hpp"
#include "rendering/sensor/transparent.hpp"
#include "rendering/sensor/clamp.inl"
#include "rendering/sensor/filtered.inl"
#include "rendering/sensor/unfiltered.inl"
#include "rendering/sensor/filter/gaussian.hpp"
#include "rendering/sensor/tonemapping/aces.hpp"
#include "rendering/sensor/tonemapping/identity.hpp"
#include "rendering/sensor/tonemapping/uncharted.hpp"
#include "rendering/integrator/surface/ao.hpp"
#include "rendering/integrator/surface/normal.hpp"
#include "rendering/integrator/surface/whitted.hpp"
#include "rendering/integrator/surface/pathtracer.hpp"
#include "rendering/integrator/surface/pathtracer_dl.hpp"
#include "rendering/integrator/surface/pathtracer_mis.hpp"
#include "rendering/integrator/volume/attenuation.hpp"
#include "rendering/integrator/volume/single_scattering.hpp"
#include "sampler/sampler_ems.hpp"
#include "sampler/sampler_ld.hpp"
#include "sampler/sampler_random.hpp"
#include "sampler/sampler_scrambled_hammersley.hpp"
#include "sampler/sampler_uniform.hpp"
#include "scene/animation/animation_loader.hpp"
#include "scene/camera/camera_cubic.hpp"
#include "scene/camera/camera_cubic_stereoscopic.hpp"
#include "scene/camera/camera_hemispherical.hpp"
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

void load_focus(const json::Value& focus_value, scene::camera::Perspective::Focus& focus);

std::shared_ptr<Take> Loader::load(std::istream& stream) {
	auto root = json::parse(stream);

	auto take = std::make_shared<Take>();

	const json::Value* exporter_value = nullptr;
	bool alpha_transparency = peek_alpha_transparency(*root);

	for (auto n = root->MemberBegin(); n != root->MemberEnd(); ++n) {
		const std::string node_name = n->name.GetString();
		const json::Value& node_value = n->value;

		if ("camera" == node_name) {
			load_camera(node_value, alpha_transparency, *take);
		} else if ("export" == node_name) {
			exporter_value = &node_value;
		} else if ("start_frame" == node_name) {
			take->view.start_frame = json::read_uint(node_value);
		} else if ("num_frames" == node_name) {
			take->view.num_frames = json::read_uint(node_value);
		} else if ("integrator" == node_name) {
			load_integrator_factories(node_value, *take);
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

	if (take->view.camera) {
		if (exporter_value) {
			take->exporter = load_exporter(*exporter_value, *take->view.camera);
		}

		if (!take->exporter) {
			image::Writer* writer = new image::encoding::png::Writer(
						take->view.camera->sensor().dimensions());
			take->exporter = std::make_unique<exporting::Image_sequence>("output_", writer);
			logging::warning("No exporter was specified, defaulting to PNG writer.");
		}
	}

	if (!take->sampler) {
		take->sampler = std::make_shared<sampler::Random>(take->rng, 1);
	}

	if (!take->surface_integrator_factory) {
		take->surface_integrator_factory = std::make_shared<
				rendering::integrator::surface::Pathtracer_MIS_factory>(
					take->settings, 4, 8, 0.5f, 1, false);
		logging::warning("No surface integrator specified, "
						 "defaulting to Pathtracer Multiple Importance Sampling.");
	}

	if (!take->volume_integrator_factory) {
		take->volume_integrator_factory = std::make_shared<
				rendering::integrator::volume::Attenuation_factory>(take->settings);
	}

	return take;
}

void Loader::load_camera(const json::Value& camera_value, bool alpha_transparency,
						 Take& take) const {
	std::string type_name = "Perspective";
	const json::Value* type_value = nullptr;

	for (auto n = camera_value.MemberBegin(); n != camera_value.MemberEnd(); ++n) {
		type_name = n->name.GetString();
		type_value = &n->value;
	}

	math::transformation transformation{
		math::float3_identity,
		math::float3(1.f, 1.f, 1.f),
		math::quaternion_identity
	};

	const json::Value* animation_value = nullptr;
	const json::Value* sensor_value = nullptr;
	std::string layout_type;
	Stereoscopic stereo;
	float frame_duration = 0.f;
	bool  motion_blur = true;
	scene::camera::Perspective::Focus focus;
	float fov = math::degrees_to_radians(60.f);
	float lens_radius = 0.f;

	for (auto n = type_value->MemberBegin(); n != type_value->MemberEnd(); ++n) {
		const std::string node_name = n->name.GetString();
		const json::Value& node_value = n->value;

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
						take.settings.ray_max_t);
		} else {
			scene::camera::Cubic::Layout layout = scene::camera::Cubic::Layout::xmxymyzmz;

			if ("xmxy_myzmz" == layout_type) {
				layout = scene::camera::Cubic::Layout::xmxy_myzmz;
			}

			camera = std::make_shared<scene::camera::Cubic>(
						layout, resolution, take.settings.ray_max_t);
		}
	} else if ("Perspective" == type_name) {
		if (stereo.interpupillary_distance > 0.f) {
			camera = std::make_shared<scene::camera::Perspective_stereoscopic>(
						stereo.interpupillary_distance, resolution,
						take.settings.ray_max_t, fov);
		} else {
			camera = std::make_shared<scene::camera::Perspective>(
						resolution, take.settings.ray_max_t,
						fov, lens_radius, focus);
		}
	} else if ("Spherical" == type_name) {
		if (stereo.interpupillary_distance > 0.f) {
			camera = std::make_shared<scene::camera::Spherical_stereoscopic>(
						stereo.interpupillary_distance, resolution,
						take.settings.ray_max_t);
		} else {
			camera = std::make_shared<scene::camera::Spherical>(
						resolution, take.settings.ray_max_t);
		}
	} else if ("Hemispherical" == type_name) {
		camera = std::make_shared<scene::camera::Hemispherical>(
					resolution, take.settings.ray_max_t);
	} else {
		throw std::runtime_error("Camera type \"" + type_name + "\" not recognized");
	}

	rendering::sensor::Sensor* sensor = load_sensor(*sensor_value, camera->sensor_dimensions(),
													alpha_transparency);

	camera->set_sensor(sensor);
	camera->set_transformation(transformation);
	camera->set_frame_duration(frame_duration);
	camera->set_motion_blur(motion_blur);

	take.view.camera = camera;
}

void Loader::load_stereoscopic(const json::Value& stereo_value, Stereoscopic& stereo) const {
	for (auto n = stereo_value.MemberBegin(); n != stereo_value.MemberEnd(); ++n) {
		const std::string node_name = n->name.GetString();
		const json::Value& node_value = n->value;

		if ("interpupillary_distance" == node_name) {
			stereo.interpupillary_distance = json::read_float(node_value);
		}
	}
}

rendering::sensor::Sensor* Loader::load_sensor(const json::Value& sensor_value,
											   math::int2 dimensions,
											   bool alpha_transparency) const {
	math::float3 clamp_max(-1.f, -1.f, -1.f);
	const rendering::sensor::tonemapping::Tonemapper* tonemapper = nullptr;
	const rendering::sensor::filter::Filter* filter = nullptr;

	for (auto n = sensor_value.MemberBegin(); n != sensor_value.MemberEnd(); ++n) {
		const std::string node_name = n->name.GetString();
		const json::Value& node_value = n->value;

		if ("clamp" == node_name) {
			clamp_max = json::read_float3(node_value);
		} else if ("tonemapper" == node_name) {
			tonemapper = load_tonemapper(node_value);
		} else if ("filter" == node_name) {
			filter = load_filter(node_value);
		}
	}

	if (!tonemapper) {
		tonemapper = new rendering::sensor::tonemapping::Identity();
	}

	bool clamp = !math::contains_negative(clamp_max);

	if (filter) {
		if (alpha_transparency) {
			if (clamp) {
				return new rendering::sensor::Filtered<
						rendering::sensor::Transparent,
						rendering::sensor::clamp::Clamp>(
							dimensions, tonemapper,
							rendering::sensor::clamp::Clamp(clamp_max), filter);
			} else {
				return new rendering::sensor::Filtered<
						rendering::sensor::Transparent,
						rendering::sensor::clamp::Identity>(
							dimensions, tonemapper, rendering::sensor::clamp::Identity(), filter);
			}
		}

		if (clamp) {
			return new rendering::sensor::Filtered<
					rendering::sensor::Opaque,
					rendering::sensor::clamp::Clamp>(
						dimensions, tonemapper, rendering::sensor::clamp::Clamp(clamp_max), filter);
		} else {
			return new rendering::sensor::Filtered<
					rendering::sensor::Opaque,
					rendering::sensor::clamp::Identity>(
						dimensions, tonemapper, rendering::sensor::clamp::Identity(), filter);
		}
	}

	if (alpha_transparency) {
		if (clamp) {
			return new rendering::sensor::Unfiltered<
					rendering::sensor::Transparent,
					rendering::sensor::clamp::Clamp>
					(dimensions, tonemapper, rendering::sensor::clamp::Clamp(clamp_max));
		} else {
			return new rendering::sensor::Unfiltered<
					rendering::sensor::Transparent,
					rendering::sensor::clamp::Identity>
					(dimensions, tonemapper, rendering::sensor::clamp::Identity());
		}
	}

	if (clamp) {
		return new rendering::sensor::Unfiltered<
				rendering::sensor::Opaque,
				rendering::sensor::clamp::Clamp>
				(dimensions, tonemapper, rendering::sensor::clamp::Clamp(clamp_max));
	}

	return new rendering::sensor::Unfiltered<
			rendering::sensor::Opaque,
			rendering::sensor::clamp::Identity>
			(dimensions, tonemapper, rendering::sensor::clamp::Identity());
}

const rendering::sensor::tonemapping::Tonemapper*
Loader::load_tonemapper(const json::Value& tonemapper_value) const {
	for (auto n = tonemapper_value.MemberBegin(); n != tonemapper_value.MemberEnd(); ++n) {
		const std::string node_name = n->name.GetString();
		const json::Value& node_value = n->value;

		if ("ACES" == node_name) {
			math::float3 linear_white = json::read_float3(node_value, "linear_white");
			float exposure = json::read_float(node_value, "exposure", 0.f);
			return new rendering::sensor::tonemapping::Aces(linear_white, exposure);
		} else if ("Identity" == node_name) {
			return new rendering::sensor::tonemapping::Identity();
		} else if ("Uncharted" == node_name) {
			math::float3 linear_white = json::read_float3(node_value, "linear_white");
			float exposure = json::read_float(node_value, "exposure", 0.f);
			return new rendering::sensor::tonemapping::Uncharted(linear_white, exposure);
		}
	}

	logging::warning("A tonemapper with unknonw type was declared. Using identity tonemapper.");

	return nullptr;
}

const rendering::sensor::filter::Filter*
Loader::load_filter(const json::Value& filter_value) const {
	for (auto n = filter_value.MemberBegin(); n != filter_value.MemberEnd(); ++n) {
		const std::string node_name = n->name.GetString();
		const json::Value& node_value = n->value;

		if ("Gaussian" == node_name) {
			float radius = json::read_float(node_value, "radius", 0.8f);
			float alpha  = json::read_float(node_value, "alpha", 0.3f);

			return new rendering::sensor::filter::Gaussian(radius, alpha);
		}
	}

	logging::warning("A reconstruction filter with unknonw type was declared. "
					 "Not using any filter.");

	return nullptr;
}

std::shared_ptr<sampler::Sampler> Loader::load_sampler(const json::Value& sampler_value,
													   math::random::Generator& rng) const {
	for (auto n = sampler_value.MemberBegin(); n != sampler_value.MemberEnd(); ++n) {
		const std::string node_name = n->name.GetString();
		const json::Value& node_value = n->value;

		if ("Uniform" == node_name) {
		//   uint32_t num_samples = json::read_uint(node_value, "samples_per_pixel");
		   return std::make_shared<sampler::Uniform>(rng);
		} else if ("Random" == node_name) {
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

void Loader::load_integrator_factories(const json::Value& integrator_value, Take& take) const {
	for (auto n = integrator_value.MemberBegin(); n != integrator_value.MemberEnd(); ++n) {
		const std::string node_name = n->name.GetString();
		const json::Value& node_value = n->value;

		if ("surface" == node_name) {
			take.surface_integrator_factory = load_surface_integrator_factory(node_value,
																			  take.settings);
		} else if ("volume" == node_name) {
			take.volume_integrator_factory = load_volume_integrator_factory(node_value,
																			take.settings);
		}
	}
}

std::shared_ptr<rendering::integrator::surface::Integrator_factory>
Loader::load_surface_integrator_factory(const json::Value& integrator_value,
										const Settings& settings) const {
	uint32_t default_min_bounces = 4;
	uint32_t default_max_bounces = 8;
	uint32_t default_max_light_samples = 1;
	float default_path_termination_probability = 0.5f;
	bool default_caustics = true;

	for (auto n = integrator_value.MemberBegin(); n != integrator_value.MemberEnd(); ++n) {
		const std::string node_name = n->name.GetString();
		const json::Value& node_value = n->value;

		if ("AO" == node_name) {
			uint32_t num_samples = json::read_uint(node_value, "num_samples", 1);
			float radius = json::read_float(node_value, "radius", 1.f);
			return std::make_shared<rendering::integrator::surface::Ao_factory>(
						settings, num_samples, radius);
		} else if ("Whitted" == node_name) {
			uint32_t num_light_samples = json::read_uint(
						node_value, "num_light_samples", default_max_light_samples);

			return std::make_shared<rendering::integrator::surface::Whitted_factory>(
						settings, num_light_samples);
		} else if ("PT" == node_name) {
			uint32_t min_bounces = json::read_uint(node_value, "min_bounces", default_min_bounces);
			uint32_t max_bounces = json::read_uint(node_value, "max_bounces", default_max_bounces);

			float path_termination_probability = json::read_float(
						node_value, "path_termination_probability",
						default_path_termination_probability);

			bool disable_caustics = !json::read_bool(node_value, "caustics", default_caustics);

			return std::make_shared<rendering::integrator::surface::Pathtracer_factory>(
						settings, min_bounces, max_bounces,
						path_termination_probability, disable_caustics);
		} else if ("PTDL" == node_name) {
			uint32_t min_bounces = json::read_uint(node_value, "min_bounces", default_min_bounces);
			uint32_t max_bounces = json::read_uint(node_value, "max_bounces", default_max_bounces);

			float path_termination_probability = json::read_float(
						node_value, "path_termination_probability",
						default_path_termination_probability);

			uint32_t num_light_samples = json::read_uint(node_value, "num_light_samples",
														 default_max_light_samples);

			bool disable_caustics = !json::read_bool(node_value, "caustics", default_caustics);

			return std::make_shared<rendering::integrator::surface::Pathtracer_DL_factory>(
						settings, min_bounces, max_bounces, path_termination_probability,
						num_light_samples, disable_caustics);
		} else if ("PTMIS" == node_name) {
			uint32_t min_bounces = json::read_uint(node_value, "min_bounces", default_min_bounces);
			uint32_t max_bounces = json::read_uint(node_value, "max_bounces", default_max_bounces);

			float path_termination_probability = json::read_float(
						node_value, "path_termination_probability",
						default_path_termination_probability);

			uint32_t num_light_samples = json::read_uint(
						node_value, "num_light_samples",
						default_max_light_samples);

			bool disable_caustics = !json::read_bool(node_value, "caustics", default_caustics);

			return std::make_shared<rendering::integrator::surface::Pathtracer_MIS_factory>(
						settings, min_bounces, max_bounces, path_termination_probability,
						num_light_samples, disable_caustics);
		} else if ("Normal" == node_name) {
			auto vector = rendering::integrator::surface::Normal::Settings::Vector::Shading_normal;

			std::string vector_type = json::read_string(node_value, "vector");

			if ("Tangent" == vector_type) {
				vector = rendering::integrator::surface::Normal::Settings::Vector::Tangent;
			} else if ("Bitangent" == vector_type) {
				vector = rendering::integrator::surface::Normal::Settings::Vector::Bitangent;
			} else if ("Geometric_normal" == vector_type) {
				vector = rendering::integrator::surface::Normal::Settings::Vector::Geometric_normal;
			} else if ("Shading_normal" == vector_type) {
				vector = rendering::integrator::surface::Normal::Settings::Vector::Shading_normal;
			}

			return std::make_shared<rendering::integrator::surface::Normal_factory>(
						settings, vector);
		}
	}

	return nullptr;
}

std::shared_ptr<rendering::integrator::volume::Integrator_factory>
Loader::load_volume_integrator_factory(const json::Value& integrator_value,
									   const Settings& settings) const {
	for (auto n = integrator_value.MemberBegin(); n != integrator_value.MemberEnd(); ++n) {
		const std::string node_name = n->name.GetString();
		const json::Value& node_value = n->value;

		if ("Attenuation" == node_name) {
			return std::make_shared<rendering::integrator::volume::Attenuation_factory>(settings);
		} else if ("Single_scattering" == node_name) {
			float step_size = json::read_float(node_value, "step_size", 1.f);
			return std::make_shared<
					rendering::integrator::volume::Single_scattering_factory>(settings, step_size);
		}
	}

	return nullptr;
}

bool Loader::peek_alpha_transparency(const json::Value& take_value) const {
	const json::Value::ConstMemberIterator export_node = take_value.FindMember("export");
	if (take_value.MemberEnd() == export_node) {
		return false;
	}

	const json::Value::ConstMemberIterator node = export_node->value.FindMember("Image");
	if (export_node->value.MemberEnd() == node) {
		return false;
	}

	return json::read_bool(node->value, "alpha_transparency");
}

std::unique_ptr<exporting::Sink> Loader::load_exporter(const json::Value& exporter_value,
													   scene::camera::Camera& camera) const {
	for (auto n = exporter_value.MemberBegin(); n != exporter_value.MemberEnd(); ++n) {
		const std::string node_name = n->name.GetString();
		const json::Value& node_value = n->value;

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

			if (0 == framerate) {
				framerate = static_cast<uint32_t>(1.f / camera.frame_duration() + 0.5f);
			}

			return std::make_unique<exporting::Ffmpeg>("output",
													   camera.sensor().dimensions(), framerate);
		} else if ("Null" == node_name) {
			return std::make_unique<exporting::Null>();
		} else if ("Statistics" == node_name) {
			return std::make_unique<exporting::Statistics>();
		}
	}

	return nullptr;
}

void Loader::load_settings(const json::Value& settings_value, Settings& settings) const {
	for (auto n = settings_value.MemberBegin(); n != settings_value.MemberEnd(); ++n) {
		const std::string node_name = n->name.GetString();
		const json::Value& node_value = n->value;

		if ("ray_offset_factor" == node_name) {
			settings.ray_offset_factor = json::read_float(node_value);
		}
	}
}

void load_focus(const json::Value& focus_value, scene::camera::Perspective::Focus& focus) {
	for (auto n = focus_value.MemberBegin(); n != focus_value.MemberEnd(); ++n) {
		const std::string node_name = n->name.GetString();
		const json::Value& node_value = n->value;

		if ("point" == node_name) {
			focus.point = json::read_float3(node_value);
			focus.use_point = true;
		} else if ("distance" == node_name) {
			focus.distance = json::read_float(node_value);
		}
	}
}

}
