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

std::shared_ptr<Take> Loader::load(std::istream& stream) {
	auto root = json::parse(stream);

	auto take = std::make_shared<Take>();

	const json::Value* exporter_value = nullptr;
	bool alpha_transparency = peek_alpha_transparency(*root);

	for (auto& n : root->GetObject()) {
		if ("camera" == n.name) {
			load_camera(n.value, alpha_transparency, *take);
		} else if ("export" == n.name) {
			exporter_value = &n.value;
		} else if ("start_frame" == n.name) {
			take->view.start_frame = json::read_uint(n.value);
		} else if ("num_frames" == n.name) {
			take->view.num_frames = json::read_uint(n.value);
		} else if ("integrator" == n.name) {
			load_integrator_factories(n.value, *take);
		} else if ("sampler" == n.name) {
			take->sampler_factory = load_sampler_factory(n.value);
		} else if ("scene" == n.name) {
			take->scene_filename = n.value.GetString();
		} else if ("settings" == n.name) {
			load_settings(n.value, take->settings);
		}
	}

	if (take->scene_filename.empty()) {
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

	if (!take->sampler_factory) {
		take->sampler_factory = std::make_shared<sampler::Random_factory>(1);
	}

	if (!take->surface_integrator_factory) {
		rendering::integrator::Light_sampling light_sampling{
			rendering::integrator::Light_sampling::Strategy::One, 1};
		take->surface_integrator_factory = std::make_shared<
				rendering::integrator::surface::Pathtracer_MIS_factory>(
					take->settings, 4, 8, 0.5f, light_sampling, false);
		logging::warning("No valid surface integrator specified, defaulting to PTMIS.");
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

	for (auto& n : camera_value.GetObject()) {
		type_name = n.name.GetString();
		type_value = &n.value;
	}

	math::transformation transformation {
		math::float3_identity,
		float3(1.f, 1.f, 1.f),
		math::quaternion_identity
	};

	const json::Value* parameters_value = nullptr;
	const json::Value* animation_value = nullptr;
	const json::Value* sensor_value = nullptr;

	std::string layout_type;
	bool stereo = false;

	for (auto& n : type_value->GetObject()) {
		if ("parameters" == n.name) {
			parameters_value = &n.value;
			stereo = peek_stereoscopic(n.value);
		} else if ("transformation" == n.name) {
			json::read_transformation(n.value, transformation);
		} else if ("animation" == n.name) {
			animation_value = &n.value;
		} else if ("sensor" == n.name) {
			sensor_value = &n.value;
		} else if ("layout" == n.name) {
			layout_type = json::read_string(n.value);
		}
	}

	int2 resolution;
	if (sensor_value) {
		resolution = json::read_int2(*sensor_value, "resolution");
		if (int2::identity == resolution) {
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
		if (stereo) {
			scene::camera::Cubic_stereoscopic::Layout layout =
					scene::camera::Cubic_stereoscopic::Layout::rxlmxryrmyrzrmzlxlmxlylmylzlmz;

			if ("lxlmxlylmylzlmzrxrmxryrmyrzrmz" == layout_type) {
				layout = scene::camera::Cubic_stereoscopic::Layout::lxlmxlylmylzlmzrxrmxryrmyrzrmz;
			}

			camera = std::make_shared<scene::camera::Cubic_stereoscopic>(
						layout, resolution, take.settings.ray_max_t);
		} else {
			scene::camera::Cubic::Layout layout = scene::camera::Cubic::Layout::xmxymyzmz;

			if ("xmxy_myzmz" == layout_type) {
				layout = scene::camera::Cubic::Layout::xmxy_myzmz;
			}

			camera = std::make_shared<scene::camera::Cubic>(
						layout, resolution, take.settings.ray_max_t);
		}
	} else if ("Perspective" == type_name) {
		if (stereo) {
			camera = std::make_shared<scene::camera::Perspective_stereoscopic>(
						resolution, take.settings.ray_max_t);
		} else {
			camera = std::make_shared<scene::camera::Perspective>(
						resolution, take.settings.ray_max_t);
		}
	} else if ("Spherical" == type_name) {
		if (stereo) {
			camera = std::make_shared<scene::camera::Spherical_stereoscopic>(
						resolution, take.settings.ray_max_t);
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

	if (parameters_value) {
		camera->set_parameters(*parameters_value);
	}

	camera->set_transformation(transformation);

	if (sensor_value) {
		rendering::sensor::Sensor* sensor = load_sensor(*sensor_value,
														camera->sensor_dimensions(),
														alpha_transparency);

		camera->set_sensor(sensor);
	}

	take.view.camera = camera;
}

rendering::sensor::Sensor* Loader::load_sensor(const json::Value& sensor_value,
											   int2 dimensions,
											   bool alpha_transparency) const {
	float3 clamp_max(-1.f, -1.f, -1.f);
	const rendering::sensor::tonemapping::Tonemapper* tonemapper = nullptr;
	const rendering::sensor::filter::Filter* filter = nullptr;

	for (auto& n : sensor_value.GetObject()) {
		if ("clamp" == n.name) {
			clamp_max = json::read_float3(n.value);
		} else if ("tonemapper" == n.name) {
			tonemapper = load_tonemapper(n.value);
		} else if ("filter" == n.name) {
			filter = load_filter(n.value);
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
	for (auto& n : tonemapper_value.GetObject()) {
		if ("ACES" == n.name) {
			float3 linear_white = json::read_float3(n.value, "linear_white");
			float exposure = json::read_float(n.value, "exposure", 0.f);
			return new rendering::sensor::tonemapping::Aces(linear_white, exposure);
		} else if ("Identity" == n.name) {
			return new rendering::sensor::tonemapping::Identity();
		} else if ("Uncharted" == n.name) {
			float3 linear_white = json::read_float3(n.value, "linear_white");
			float exposure = json::read_float(n.value, "exposure", 0.f);
			return new rendering::sensor::tonemapping::Uncharted(linear_white, exposure);
		}
	}

	logging::warning("A tonemapper with unknonw type was declared. Using identity tonemapper.");

	return nullptr;
}

const rendering::sensor::filter::Filter*
Loader::load_filter(const json::Value& filter_value) const {
	for (auto& n : filter_value.GetObject()) {
		if ("Gaussian" == n.name) {
			float radius = json::read_float(n.value, "radius", 0.8f);
			float alpha  = json::read_float(n.value, "alpha", 0.3f);

			return new rendering::sensor::filter::Gaussian(radius, alpha);
		}
	}

	logging::warning("A reconstruction filter with unknonw type was declared. "
					 "Not using any filter.");

	return nullptr;
}

std::shared_ptr<sampler::Factory> Loader::load_sampler_factory(
		const json::Value& sampler_value) const {
	for (auto& n : sampler_value.GetObject()) {
		if ("Uniform" == n.name) {
		   // uint32_t num_samples = json::read_uint(node_value, "samples_per_pixel");
		   return std::make_shared<sampler::Uniform_factory>(1);
		} else if ("Random" == n.name) {
			uint32_t num_samples = json::read_uint(n.value, "samples_per_pixel");
			return std::make_shared<sampler::Random_factory>(num_samples);
		} else if ("Scrambled_hammersley" == n.name) {
			 uint32_t num_samples = json::read_uint(n.value, "samples_per_pixel");
			 return std::make_shared<sampler::Scrambled_hammersley_factory>(num_samples);
		 } else if ("EMS" == n.name) {
			 uint32_t num_samples = json::read_uint(n.value, "samples_per_pixel");
			 return std::make_shared<sampler::EMS_factory>(num_samples);
		 } else if ("LD" == n.name) {
			 uint32_t num_samples = json::read_uint(n.value, "samples_per_pixel");
			 return std::make_shared<sampler::LD_factory>(num_samples);
		 }
	}

	return nullptr;
}

void Loader::load_integrator_factories(const json::Value& integrator_value, Take& take) const {
	for (auto& n : integrator_value.GetObject()) {
		if ("surface" == n.name) {
			take.surface_integrator_factory = load_surface_integrator_factory(n.value,
																			  take.settings);
		} else if ("volume" == n.name) {
			take.volume_integrator_factory = load_volume_integrator_factory(n.value,
																			take.settings);
		}
	}
}

std::shared_ptr<rendering::integrator::surface::Factory>
Loader::load_surface_integrator_factory(const json::Value& integrator_value,
										const Settings& settings) const {
	uint32_t default_min_bounces = 4;
	uint32_t default_max_bounces = 8;
	rendering::integrator::Light_sampling light_sampling{
		rendering::integrator::Light_sampling::Strategy::One, 1};
	float default_path_termination_probability = 0.5f;
	bool default_caustics = true;

	for (auto& n : integrator_value.GetObject()) {
		if ("AO" == n.name) {
			uint32_t num_samples = json::read_uint(n.value, "num_samples", 1);
			float radius = json::read_float(n.value, "radius", 1.f);
			return std::make_shared<rendering::integrator::surface::Ao_factory>(
						settings, num_samples, radius);
		} else if ("Whitted" == n.name) {
			uint32_t num_light_samples = json::read_uint(
						n.value, "num_light_samples", light_sampling.num_samples);

			return std::make_shared<rendering::integrator::surface::Whitted_factory>(
						settings, num_light_samples);
		} else if ("PT" == n.name) {
			uint32_t min_bounces = json::read_uint(n.value, "min_bounces", default_min_bounces);
			uint32_t max_bounces = json::read_uint(n.value, "max_bounces", default_max_bounces);

			float path_termination_probability = json::read_float(
						n.value, "path_termination_probability",
						default_path_termination_probability);

			bool disable_caustics = !json::read_bool(n.value, "caustics", default_caustics);

			return std::make_shared<rendering::integrator::surface::Pathtracer_factory>(
						settings, min_bounces, max_bounces,
						path_termination_probability, disable_caustics);
		} else if ("PTDL" == n.name) {
			uint32_t min_bounces = json::read_uint(n.value, "min_bounces", default_min_bounces);
			uint32_t max_bounces = json::read_uint(n.value, "max_bounces", default_max_bounces);

			float path_termination_probability = json::read_float(
						n.value, "path_termination_probability",
						default_path_termination_probability);


			uint32_t num_light_samples = json::read_uint(n.value, "num_light_samples",
														 light_sampling.num_samples);

			bool disable_caustics = !json::read_bool(n.value, "caustics", default_caustics);

			return std::make_shared<rendering::integrator::surface::Pathtracer_DL_factory>(
						settings, min_bounces, max_bounces, path_termination_probability,
						num_light_samples, disable_caustics);
		} else if ("PTMIS" == n.name) {
			uint32_t min_bounces = json::read_uint(n.value, "min_bounces", default_min_bounces);
			uint32_t max_bounces = json::read_uint(n.value, "max_bounces", default_max_bounces);

			float path_termination_probability = json::read_float(
						n.value, "path_termination_probability",
						default_path_termination_probability);

			const auto light_sampling_node = n.value.FindMember("light_sampling");
			if (n.value.MemberEnd() != light_sampling_node) {
				load_light_sampling(light_sampling_node->value, light_sampling);
			}

			bool disable_caustics = !json::read_bool(n.value, "caustics", default_caustics);

			return std::make_shared<rendering::integrator::surface::Pathtracer_MIS_factory>(
						settings, min_bounces, max_bounces, path_termination_probability,
						light_sampling, disable_caustics);
		} else if ("Normal" == n.name) {
			auto vector = rendering::integrator::surface::Normal::Settings::Vector::Shading_normal;

			std::string vector_type = json::read_string(n.value, "vector");

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

std::shared_ptr<rendering::integrator::volume::Factory>
Loader::load_volume_integrator_factory(const json::Value& integrator_value,
									   const Settings& settings) const {
	for (auto& n : integrator_value.GetObject()) {
		const std::string node_name = n.name.GetString();

		if ("Attenuation" == node_name) {
			return std::make_shared<rendering::integrator::volume::Attenuation_factory>(settings);
		} else if ("Single_scattering" == node_name) {
			float step_size = json::read_float(n.value, "step_size", 1.f);
			return std::make_shared<
					rendering::integrator::volume::Single_scattering_factory>(settings, step_size);
		}
	}

	return nullptr;
}

bool Loader::peek_alpha_transparency(const json::Value& take_value) {
	const auto export_node = take_value.FindMember("export");
	if (take_value.MemberEnd() == export_node) {
		return false;
	}

	const auto node = export_node->value.FindMember("Image");
	if (export_node->value.MemberEnd() == node) {
		return false;
	}

	return json::read_bool(node->value, "alpha_transparency");
}

bool Loader::peek_stereoscopic(const json::Value& parameters_value) {
	const auto export_node = parameters_value.FindMember("stereo");
	if (parameters_value.MemberEnd() == export_node) {
		return false;
	}

	return true;
}

std::unique_ptr<exporting::Sink> Loader::load_exporter(const json::Value& exporter_value,
													   scene::camera::Camera& camera) {
	for (auto& n : exporter_value.GetObject()) {
		const std::string node_name = n.name.GetString();

		if ("Image" == node_name) {
			const std::string format = json::read_string(n.value, "format", "PNG");

			image::Writer* writer;

			if ("RGBE" == format) {
				writer = new image::encoding::rgbe::Writer;
			} else {
				writer = new image::encoding::png::Writer(camera.sensor().dimensions());
			}

			return std::make_unique<exporting::Image_sequence>("output_", writer);
		} else if ("Movie" == node_name) {
			uint32_t framerate = json::read_uint(n.value, "framerate");

			if (0 == framerate) {
				framerate = static_cast<uint32_t>(1.f / camera.frame_duration() + 0.5f);
			}

			return std::make_unique<exporting::Ffmpeg>("output", camera.sensor().dimensions(),
													   framerate);
		} else if ("Null" == node_name) {
			return std::make_unique<exporting::Null>();
		} else if ("Statistics" == node_name) {
			return std::make_unique<exporting::Statistics>();
		}
	}

	return nullptr;
}

void Loader::load_settings(const json::Value& settings_value, Settings& settings) {
	for (auto& n : settings_value.GetObject()) {
		const std::string node_name = n.name.GetString();

		if ("ray_offset_factor" == node_name) {
			settings.ray_offset_factor = json::read_float(n.value);
		}
	}
}

void Loader::load_light_sampling(const json::Value& sampling_value,
								 rendering::integrator::Light_sampling& sampling) {
	for (auto& n : sampling_value.GetObject()) {
		const std::string node_name = n.name.GetString();

		if ("strategy" == n.name) {
			std::string strategy = json::read_string(n.value);

			if ("One" == strategy) {
				sampling.strategy = rendering::integrator::Light_sampling::Strategy::One;
			} else if ("All" == strategy) {
				sampling.strategy = rendering::integrator::Light_sampling::Strategy::All;
			}
		} else if ("num_samples" == node_name) {
			sampling.num_samples = json::read_uint(n.value);
		}
	}
}

}
