#include "take_loader.hpp"
#include "take.hpp"
#include "exporting/exporting_sink_ffmpeg.hpp"
#include "exporting/exporting_sink_image_sequence.hpp"
#include "exporting/exporting_sink_null.hpp"
#include "exporting/exporting_sink_statistics.hpp"
#include "image/encoding/png/png_writer.hpp"
#include "image/encoding/rgbe/rgbe_writer.hpp"
#include "logging/logging.hpp"
#include "rendering/integrator/surface/ao.hpp"
#include "rendering/integrator/surface/debug.hpp"
#include "rendering/integrator/surface/whitted.hpp"
#include "rendering/integrator/surface/pathtracer.hpp"
#include "rendering/integrator/surface/pathtracer_dl.hpp"
#include "rendering/integrator/surface/pathtracer_mis.hpp"
#include "rendering/integrator/volume/attenuation.hpp"
#include "rendering/integrator/volume/single_scattering.hpp"
#include "rendering/postprocessor/postprocessor_bloom.hpp"
#include "rendering/postprocessor/postprocessor_glare.hpp"
#include "rendering/postprocessor/postprocessor_glare2.hpp"
#include "rendering/postprocessor/tonemapping/aces.hpp"
#include "rendering/postprocessor/tonemapping/generic.hpp"
#include "rendering/postprocessor/tonemapping/identity.hpp"
#include "rendering/postprocessor/tonemapping/uncharted.hpp"
#include "rendering/sensor/opaque.hpp"
#include "rendering/sensor/transparent.hpp"
#include "rendering/sensor/clamp.inl"
#include "rendering/sensor/filtered.inl"
#include "rendering/sensor/unfiltered.inl"
#include "rendering/sensor/filter/sensor_gaussian.hpp"
#include "sampler/sampler_ems.hpp"
#include "sampler/sampler_golden_ratio.hpp"
#include "sampler/sampler_ld.hpp"
#include "sampler/sampler_random.hpp"
#include "sampler/sampler_hammersley.hpp"
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
#include "base/math/vector3.inl"
#include "base/math/matrix3x3.inl"
#include "base/math/quaternion.inl"
#include "base/json/json.hpp"
#include "base/thread/thread_pool.hpp"

namespace take {

std::shared_ptr<Take> Loader::load(std::istream& stream, thread::Pool& thread_pool) {
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
			load_integrator_factories(n.value, thread_pool.num_threads(), *take);
		} else if ("postprocessors" == n.name) {
			load_postprocessors(n.value, *take);
		} else if ("sampler" == n.name) {
			take->sampler_factory = load_sampler_factory(n.value, take->view.num_samples_per_pixel);
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
			const auto d = take->view.camera->sensor().dimensions();
			image::Writer* writer = new image::encoding::png::Writer(d);
			take->exporter = std::make_unique<exporting::Image_sequence>("output_", writer);
			logging::warning("No exporter was specified, defaulting to PNG writer.");
		}
	}

	if (!take->sampler_factory) {
		take->sampler_factory = std::make_shared<sampler::Random_factory>();
		logging::warning("No known sampler was specified, defaulting to Random sampler.");
	}

	using namespace rendering::integrator;

	if (!take->surface_integrator_factory) {
		Light_sampling light_sampling{Light_sampling::Strategy::One, 1};
		take->surface_integrator_factory = std::make_unique<
				surface::Pathtracer_MIS_factory>(take->settings, thread_pool.num_threads(),
												 4, 8, 0.5f, light_sampling, false);
		logging::warning("No valid surface integrator specified, defaulting to PTMIS.");
	}

	if (!take->volume_integrator_factory) {
		take->volume_integrator_factory = std::make_shared<
				volume::Attenuation_factory>(take->settings);
	}

	take->view.init(thread_pool);

	return take;
}

void Loader::load_camera(const json::Value& camera_value, bool alpha_transparency, Take& take) {
	using namespace scene::camera;

	std::string type_name = "Perspective";
	const json::Value* type_value = nullptr;

	for (auto& n : camera_value.GetObject()) {
		type_name = n.name.GetString();
		type_value = &n.value;
	}

	math::Transformation transformation {
		float3::identity(),
		float3(1.f),
		math::quaternion::identity()
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
		if (int2::identity() == resolution) {
			throw std::runtime_error("Sensor resolution is [0, 0]");
		}
	} else {
		throw std::runtime_error("No sensor configuration included");
	}

	if (animation_value) {
		take.camera_animation = scene::animation::load(*animation_value, transformation);
	}

	std::shared_ptr<Camera> camera;

	if ("Cubic" == type_name) {
		if (stereo) {
			Cubic_stereoscopic::Layout layout =
					Cubic_stereoscopic::Layout::rxlmxryrmyrzrmzlxlmxlylmylzlmz;

			if ("lxlmxlylmylzlmzrxrmxryrmyrzrmz" == layout_type) {
				layout = Cubic_stereoscopic::Layout::lxlmxlylmylzlmzrxrmxryrmyrzrmz;
			}

			camera = std::make_shared<Cubic_stereoscopic>(layout, resolution);
		} else {
			Cubic::Layout layout = Cubic::Layout::xmxymyzmz;

			if ("xmxy_myzmz" == layout_type) {
				layout = Cubic::Layout::xmxy_myzmz;
			}

			camera = std::make_shared<Cubic>(layout, resolution);
		}
	} else if ("Perspective" == type_name) {
		if (stereo) {
			camera = std::make_shared<Perspective_stereoscopic>(resolution);
		} else {
			camera = std::make_shared<Perspective>(resolution);
		}
	} else if ("Spherical" == type_name) {
		if (stereo) {
			camera = std::make_shared<Spherical_stereoscopic>(resolution);
		} else {
			camera = std::make_shared<Spherical>(resolution);
		}
	} else if ("Hemispherical" == type_name) {
		camera = std::make_shared<Hemispherical>(resolution);
	} else {
		throw std::runtime_error("Camera type \"" + type_name + "\" not recognized");
	}

	if (parameters_value) {
		camera->set_parameters(*parameters_value);
	}

	camera->set_transformation(transformation);

	if (sensor_value) {
		auto sensor = load_sensor(*sensor_value, camera->sensor_dimensions(), alpha_transparency);

		camera->set_sensor(sensor);
	}

	take.view.camera = camera;
}

rendering::sensor::Sensor* Loader::load_sensor(const json::Value& sensor_value,
											   int2 dimensions,
											   bool alpha_transparency) {
	using namespace rendering::sensor;

	float exposure = 0.f;
	float3 clamp_max(-1.f);
	const filter::Filter* filter = nullptr;

	for (auto& n : sensor_value.GetObject()) {
		if ("exposure" == n.name) {
			exposure = json::read_float(n.value);
		} else if ("clamp" == n.name) {
			clamp_max = json::read_float3(n.value);
		} else if ("filter" == n.name) {
			filter = load_filter(n.value);
		}
	}

	bool clamp = !math::any_negative(clamp_max);

	if (filter) {
		if (alpha_transparency) {
			if (clamp) {
				return new Filtered<Transparent, clamp::Clamp>(dimensions, exposure,
															   clamp::Clamp(clamp_max), filter);
			} else {
				return new Filtered<Transparent, clamp::Identity>(dimensions, exposure,
																  clamp::Identity(), filter);
			}
		}

		if (clamp) {
			return new Filtered<Opaque, clamp::Clamp>(dimensions, exposure,
													  clamp::Clamp(clamp_max), filter);
		} else {
			return new Filtered<Opaque, clamp::Identity>(dimensions, exposure,
														 clamp::Identity(), filter);
		}
	}

	if (alpha_transparency) {
		if (clamp) {
			return new Unfiltered<Transparent, clamp::Clamp>(dimensions, exposure,
															 clamp::Clamp(clamp_max));
		} else {
			return new Unfiltered<Transparent, clamp::Identity>(dimensions, exposure,
																clamp::Identity());
		}
	}

	if (clamp) {
		return new Unfiltered<Opaque, clamp::Clamp>(dimensions, exposure,
													clamp::Clamp(clamp_max));
	}

	return new Unfiltered<Opaque, clamp::Identity>(dimensions, exposure, clamp::Identity());
}

const rendering::sensor::filter::Filter*
Loader::load_filter(const json::Value& filter_value) {
	for (auto& n : filter_value.GetObject()) {
		if ("Gaussian" == n.name) {
			float radius = json::read_float(n.value, "radius", 1.f);
			float alpha  = json::read_float(n.value, "alpha",  1.8f);

			return new rendering::sensor::filter::Gaussian(radius, alpha);
		}
	}

	logging::warning("A reconstruction filter with unknonw type was declared. "
					 "Not using any filter.");

	return nullptr;
}

std::shared_ptr<sampler::Factory>
Loader::load_sampler_factory(const json::Value& sampler_value, uint32_t& num_samples_per_pixel) {
	for (auto& n : sampler_value.GetObject()) {
		num_samples_per_pixel = json::read_uint(n.value, "samples_per_pixel");

		if ("Uniform" == n.name) {
			num_samples_per_pixel = 1;
			return std::make_shared<sampler::Uniform_factory>();
		} else if ("Random" == n.name) {
			return std::make_shared<sampler::Random_factory>();
		} else if ("Hammersley" == n.name) {
			return std::make_shared<sampler::Hammersley_factory>();
		} else if ("EMS" == n.name) {
			return std::make_shared<sampler::EMS_factory>();
		} else if ("Golden_ratio" == n.name) {
			return std::make_shared<sampler::Golden_ratio_factory>();
		} else if ("LD" == n.name) {
			return std::make_shared<sampler::LD_factory>();
		}
	}

	return nullptr;
}

void Loader::load_integrator_factories(const json::Value& integrator_value,
									   uint32_t num_workers, Take& take) {
	for (auto& n : integrator_value.GetObject()) {
		if ("surface" == n.name) {
			take.surface_integrator_factory = load_surface_integrator_factory(n.value,
																			  take.settings,
																			  num_workers);
		} else if ("volume" == n.name) {
			take.volume_integrator_factory = load_volume_integrator_factory(n.value,
																			take.settings);
		}
	}
}

std::shared_ptr<rendering::integrator::surface::Factory>
Loader::load_surface_integrator_factory(const json::Value& integrator_value,
										const Settings& settings, uint32_t num_workers) {
	using namespace rendering::integrator;
	using namespace rendering::integrator::surface;

	uint32_t default_min_bounces = 4;
	uint32_t default_max_bounces = 8;
	Light_sampling light_sampling{Light_sampling::Strategy::One, 1};
	float default_path_termination_probability = 0.5f;
	bool default_caustics = true;

	for (auto& n : integrator_value.GetObject()) {
		if ("AO" == n.name) {
			uint32_t num_samples = json::read_uint(n.value, "num_samples", 1);
			float radius = json::read_float(n.value, "radius", 1.f);
			return std::make_shared<AO_factory>(settings, num_workers, num_samples, radius);
		} else if ("Whitted" == n.name) {
			uint32_t num_light_samples = json::read_uint(
						n.value, "num_light_samples", light_sampling.num_samples);

			return std::make_shared<Whitted_factory>(settings, num_workers, num_light_samples);
		} else if ("PT" == n.name) {
			uint32_t min_bounces = json::read_uint(n.value, "min_bounces", default_min_bounces);
			uint32_t max_bounces = json::read_uint(n.value, "max_bounces", default_max_bounces);

			float path_termination_probability = json::read_float(
						n.value, "path_termination_probability",
						default_path_termination_probability);

			bool enable_caustics = json::read_bool(n.value, "caustics", default_caustics);

			return std::make_shared<Pathtracer_factory>(
						settings, num_workers, min_bounces, max_bounces,
						path_termination_probability, enable_caustics);
		} else if ("PTDL" == n.name) {
			uint32_t min_bounces = json::read_uint(n.value, "min_bounces", default_min_bounces);
			uint32_t max_bounces = json::read_uint(n.value, "max_bounces", default_max_bounces);

			float path_termination_probability = json::read_float(
						n.value, "path_termination_probability",
						default_path_termination_probability);

			uint32_t num_light_samples = json::read_uint(n.value, "num_light_samples",
														 light_sampling.num_samples);

			bool enable_caustics = json::read_bool(n.value, "caustics", default_caustics);

			return std::make_shared<Pathtracer_DL_factory>(
						settings, num_workers, min_bounces, max_bounces,
						path_termination_probability, num_light_samples, enable_caustics);
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

			bool enable_caustics = json::read_bool(n.value, "caustics", default_caustics);

			return std::make_shared<Pathtracer_MIS_factory>(
						settings, num_workers, min_bounces, max_bounces,
						path_termination_probability, light_sampling, enable_caustics);
		} else if ("Debug" == n.name) {
			auto vector = Debug::Settings::Vector::Shading_normal;

			std::string vector_type = json::read_string(n.value, "vector");

			if ("Tangent" == vector_type) {
				vector = Debug::Settings::Vector::Tangent;
			} else if ("Bitangent" == vector_type) {
				vector = Debug::Settings::Vector::Bitangent;
			} else if ("Geometric_normal" == vector_type) {
				vector = Debug::Settings::Vector::Geometric_normal;
			} else if ("Shading_normal" == vector_type) {
				vector = Debug::Settings::Vector::Shading_normal;
			} else if ("UV" == vector_type) {
				vector = Debug::Settings::Vector::UV;
			}

			return std::make_shared<Debug_factory>(settings, num_workers, vector);
		}
	}

	return nullptr;
}

std::shared_ptr<rendering::integrator::volume::Factory>
Loader::load_volume_integrator_factory(const json::Value& integrator_value,
									   const Settings& settings) {
	using namespace rendering::integrator::volume;

	for (auto& n : integrator_value.GetObject()) {
		if ("Attenuation" == n.name) {
			return std::make_shared<Attenuation_factory>(settings);
		} else if ("Single_scattering" == n.name) {
			float step_size = json::read_float(n.value, "step_size", 1.f);
			return std::make_shared<Single_scattering_factory>(settings, step_size);
		}
	}

	return nullptr;
}

void Loader::load_postprocessors(const json::Value& pp_value, Take& take) {
	if (!pp_value.IsArray()) {
		return;
	}

	using namespace rendering::postprocessor;

	auto& pipeline = take.view.pipeline;

	pipeline.reserve(pp_value.Size());

	for (auto& pp : pp_value.GetArray()) {
		const auto n = pp.MemberBegin();

		if ("tonemapper" == n->name) {
			pipeline.add(load_tonemapper(n->value));
		} else if ("Bloom" == n->name) {
			float angle		= json::read_float(n->value, "angle", 0.05f);
			float alpha		= json::read_float(n->value, "alpha", 0.005f);
			float threshold = json::read_float(n->value, "threshold", 2.f);
			float intensity = json::read_float(n->value, "intensity", 0.1f);
			pipeline.add(new Bloom(angle, alpha, threshold, intensity));
		} else if ("Glare" == n->name) {
			Glare2::Adaption adaption = Glare2::Adaption::Mesopic;

			std::string adaption_name = json::read_string(n->value, "adaption");
			if ("Scotopic" == adaption_name) {
				adaption = Glare2::Adaption::Scotopic;
			} else if ("Mesopic" == adaption_name) {
				adaption = Glare2::Adaption::Mesopic;
			} else if ("Photopic" == adaption_name) {
				adaption = Glare2::Adaption::Photopic;
			}

			float threshold = json::read_float(n->value, "threshold", 2.f);
			float intensity = json::read_float(n->value, "intensity", 1.f);
			pipeline.add(new Glare2(adaption, threshold, intensity));
		}
	}
}

rendering::postprocessor::tonemapping::Tonemapper*
Loader::load_tonemapper(const json::Value& tonemapper_value) {
	using namespace rendering::postprocessor::tonemapping;

	for (auto& n : tonemapper_value.GetObject()) {
		if ("ACES" == n.name) {
			float hdr_max = json::read_float(n.value, "hdr_max", 1.f);

			return new Aces(hdr_max);
		} else if ("Generic" == n.name) {
			float contrast = json::read_float(n.value, "contrast", 1.f);
			float shoulder = json::read_float(n.value, "shoulder", 1.f);
			float mid_in   = json::read_float(n.value, "mid_in",   0.18f);
			float mid_out  = json::read_float(n.value, "mid_out",  0.18f);
			float hdr_max  = json::read_float(n.value, "hdr_max",  1.f);

			return new Generic(contrast, shoulder, mid_in, mid_out, hdr_max);
		} else if ("Identity" == n.name) {
			return new rendering::postprocessor::tonemapping::Identity();
		} else if ("Uncharted" == n.name) {
			float hdr_max = json::read_float(n.value, "hdr_max", 1.f);

			return new Uncharted(hdr_max);
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
		if ("Image" == n.name) {
			const std::string format = json::read_string(n.value, "format", "PNG");

			image::Writer* writer;

			if ("RGBE" == format) {
				writer = new image::encoding::rgbe::Writer;
			} else {
				writer = new image::encoding::png::Writer(camera.sensor().dimensions());
			}

			return std::make_unique<exporting::Image_sequence>("output_", writer);
		} else if ("Movie" == n.name) {
			uint32_t framerate = json::read_uint(n.value, "framerate");

			if (0 == framerate) {
				framerate = static_cast<uint32_t>(1.f / camera.frame_duration() + 0.5f);
			}

			return std::make_unique<exporting::Ffmpeg>("output", camera.sensor().dimensions(),
													   framerate);
		} else if ("Null" == n.name) {
			return std::make_unique<exporting::Null>();
		} else if ("Statistics" == n.name) {
			return std::make_unique<exporting::Statistics>();
		}
	}

	return nullptr;
}

void Loader::load_settings(const json::Value& settings_value, Settings& settings) {
	for (auto& n : settings_value.GetObject()) {
		if ("ray_offset_factor" == n.name) {
			settings.ray_offset_factor = json::read_float(n.value);
		}
	}
}

void Loader::load_light_sampling(const json::Value& sampling_value,
								 rendering::integrator::Light_sampling& sampling) {
	for (auto& n : sampling_value.GetObject()) {
		if ("strategy" == n.name) {
			std::string strategy = json::read_string(n.value);

			if ("One" == strategy) {
				sampling.strategy = rendering::integrator::Light_sampling::Strategy::One;
			} else if ("All" == strategy) {
				sampling.strategy = rendering::integrator::Light_sampling::Strategy::All;
			}
		} else if ("num_samples" == n.name) {
			sampling.num_samples = json::read_uint(n.value);
		}
	}
}

}
