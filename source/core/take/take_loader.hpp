#pragma once

#include "rendering/integrator/integrator.hpp"
#include "base/json/json_types.hpp"
#include "base/math/vector.hpp"
#include <istream>
#include <string>
#include <memory>

namespace exporting { class Sink; }

namespace math { namespace random { class Generator; }}

namespace scene { namespace camera { class Camera; } }

namespace sampler { class Factory; }

namespace rendering {

struct Focus;

namespace sensor {

class Sensor;

namespace filter { class Filter; }

}

namespace integrator {

namespace surface { class Factory; }
namespace volume  { class Factory; }

}

namespace postprocessor {  namespace tonemapping { class Tonemapper; } }

}

namespace take {

struct Take;
struct Settings;

class Loader {

public:

	static std::shared_ptr<Take> load(std::istream& stream);

private:

	struct Stereoscopic {
		float interpupillary_distance = 0.f;
	};

	static void load_camera(const json::Value& camera_value, bool alpha_transparency, Take& take);

	static rendering::sensor::Sensor*
	load_sensor(const json::Value& sensor_value, int2 dimensions, bool alpha_transparency);

	static const rendering::sensor::filter::Filter*
	load_filter(const rapidjson::Value& filter_value);

	static std::shared_ptr<sampler::Factory>
	load_sampler_factory(const json::Value& sampler_value);

	static void load_integrator_factories(const json::Value& integrator_value, Take& take);

	static std::shared_ptr<rendering::integrator::surface::Factory>
	load_surface_integrator_factory(const json::Value& integrator_value,
									const Settings& settings);

	static std::shared_ptr<rendering::integrator::volume::Factory>
	load_volume_integrator_factory(const json::Value& integrator_value,
								   const Settings& settings);

	static void load_postprocessors(const json::Value& pp_value, Take& take);

	static rendering::postprocessor::tonemapping::Tonemapper*
	load_tonemapper(const json::Value& tonemapper_value);

	static bool peek_alpha_transparency(const json::Value& take_value);

	static bool peek_stereoscopic(const json::Value& parameters_value);

	static std::unique_ptr<exporting::Sink>
	load_exporter(const json::Value& exporter_value, scene::camera::Camera& camera);

	static void load_settings(const json::Value& settings_value, Settings& settings);

	static void load_light_sampling(const json::Value& sampling_value,
									rendering::integrator::Light_sampling& sampling);
};

}
