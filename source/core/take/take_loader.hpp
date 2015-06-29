#pragma once

#include "base/json/rapidjson_types.hpp"
#include <string>
#include <memory>

namespace exporting {

class Sink;

}

namespace math { namespace random {

class Generator;

}}

namespace scene { namespace camera {

class Camera;

}}

namespace sampler {

class Sampler;

}

namespace rendering {

namespace film {

class Film;

namespace tonemapping {

class Tonemapper;

}

namespace filter {

class Filter;

}}

class Surface_integrator_factory;

}

namespace take {

struct Take;
struct Settings;

class Loader {
public:

	std::shared_ptr<Take> load(const std::string& filename);

private:

	std::shared_ptr<scene::camera::Camera> load_camera(const rapidjson::Value& camera_value) const;

	rendering::film::Film* load_film(const rapidjson::Value& film_value) const;

	rendering::film::tonemapping::Tonemapper* load_tonemapper(const rapidjson::Value& tonemapper_value) const;

	rendering::film::filter::Filter* load_filter(const rapidjson::Value& film_value) const;

	std::shared_ptr<sampler::Sampler> load_sampler(const rapidjson::Value& sampler_value, math::random::Generator& rng) const;

	std::shared_ptr<rendering::Surface_integrator_factory> load_surface_integrator_factory(const rapidjson::Value& integrator_value,
																						   const Settings& settings) const;

	std::shared_ptr<exporting::Sink> load_exporter(const rapidjson::Value& exporter_value, scene::camera::Camera& camera) const;

	void load_settings(const rapidjson::Value& settings_value, Settings& settings) const;
};

}
