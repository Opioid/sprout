#pragma once

#include "base/json/json_types.hpp"
#include "base/math/vector.hpp"
#include <istream>
#include <string>
#include <memory>

namespace exporting { class Sink; }

namespace math { namespace random { class Generator; }}

namespace scene { namespace camera { class Camera; } }

namespace sampler { class Sampler; }

namespace rendering {

struct Focus;

namespace sensor {

class Sensor;

namespace tonemapping { class Tonemapper; }

namespace filter { class Filter; }

}

namespace integrator {

namespace surface { class Integrator_factory; }
namespace volume  { class Integrator_factory; }

}}

namespace take {

struct Take;
struct Settings;

class Loader {

public:

	std::shared_ptr<Take> load(std::istream& stream);

private:

	struct Stereoscopic {
		float interpupillary_distance = 0.f;
	};

	void load_camera(const json::Value& camera_value, bool alpha_transparency, Take& take) const;

	void load_stereoscopic(const json::Value& stereo_value, Stereoscopic& stereo) const;

	rendering::sensor::Sensor* load_sensor(const json::Value& sensor_value,
										   int2 dimensions,
										   bool alpha_transparency) const;

	const rendering::sensor::tonemapping::Tonemapper*
	load_tonemapper(const json::Value& tonemapper_value) const;

	const rendering::sensor::filter::Filter*
	load_filter(const rapidjson::Value& filter_value) const;

	std::shared_ptr<sampler::Sampler> load_sampler(const json::Value& sampler_value,
												   math::random::Generator& rng) const;

	void load_integrator_factories(const json::Value& integrator_value, Take& take) const;

	std::shared_ptr<rendering::integrator::surface::Integrator_factory>
	load_surface_integrator_factory(const json::Value& integrator_value,
									const Settings& settings) const;

	std::shared_ptr<rendering::integrator::volume::Integrator_factory>
	load_volume_integrator_factory(const json::Value& integrator_value,
								   const Settings& settings) const;

	bool peek_alpha_transparency(const json::Value& take_value) const;

	std::unique_ptr<exporting::Sink> load_exporter(const json::Value& exporter_value,
												   scene::camera::Camera& camera) const;

	void load_settings(const json::Value& settings_value, Settings& settings) const;
};

}
