#ifndef SU_CORE_TAKE_LOADER_HPP
#define SU_CORE_TAKE_LOADER_HPP

#include "base/json/json_types.hpp"
#include "base/math/vector.hpp"
#include <iosfwd>
#include <string>
#include <memory>
#include <vector>

namespace exporting { class Sink; }

namespace math::random { class Generator; }

namespace resource { class Manager; }

namespace scene::camera { class Camera; }

namespace sampler { class Factory; }

namespace rendering {
	struct Focus;

	namespace sensor {
		class Sensor;

		namespace filter { class Filter; }
	}

	namespace integrator {
		namespace surface {
			class Factory;

			namespace sub { class Factory; }
		}

		namespace volume  { class Factory; }

		struct Light_sampling;
	}

	namespace postprocessor { class Postprocessor; }
}

namespace take {

struct Take;
struct View;
struct Settings;

class Loader {

public:

	static std::unique_ptr<Take> load(std::istream& stream, resource::Manager& manager);

private:

	struct Stereoscopic {
		float interpupillary_distance = 0.f;
	};

	static void load_camera(const json::Value& camera_value, Take& take);

	static std::unique_ptr<rendering::sensor::Sensor>
	load_sensor(const json::Value& sensor_value, int2 dimensions);

	static const rendering::sensor::filter::Filter*
	load_filter(const rapidjson::Value& filter_value);

	static std::shared_ptr<sampler::Factory>
	load_sampler_factory(const json::Value& sampler_value, uint32_t num_workers,
						 uint32_t& num_samples_per_pixel);

	static void load_integrator_factories(const json::Value& integrator_value,
										  uint32_t num_workers, Take& take);

	static std::shared_ptr<rendering::integrator::surface::Factory>
	load_surface_integrator_factory(const json::Value& integrator_value,
									const Settings& settings, uint32_t num_workers);

	static std::unique_ptr<rendering::integrator::surface::sub::Factory>
	find_subsurface_integrator_factory(const json::Value& parent_value,
									   const Settings& settings, uint32_t num_workers);

	static std::unique_ptr<rendering::integrator::surface::sub::Factory>
	load_subsurface_integrator_factory(const json::Value& integrator_value,
									   const Settings& settings, uint32_t num_workers);

	static std::shared_ptr<rendering::integrator::volume::Factory>
	load_volume_integrator_factory(const json::Value& integrator_value,
								   const Settings& settings, uint32_t num_workers);

	static void load_postprocessors(const json::Value& pp_value, resource::Manager& manager,
									Take& take);

	static std::unique_ptr<rendering::postprocessor::Postprocessor>
	load_tonemapper(const json::Value& tonemapper_value);

	static bool peek_stereoscopic(const json::Value& parameters_value);

	static std::vector<std::unique_ptr<exporting::Sink>>
	load_exporters(const json::Value& exporter_value, const View& view);

	static void load_settings(const json::Value& settings_value, Settings& settings);

	static void load_light_sampling(const json::Value& parent_value,
									rendering::integrator::Light_sampling& sampling);
};

}

#endif
