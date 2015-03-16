#pragma once

#include "base/json/rapidjson_types.hpp"
#include <string>
#include <memory>

namespace camera {

class Camera;

}

namespace film {

class Film;

}

namespace rendering {

class Surface_integrator_factory;

}

namespace take {

struct Take;

class Loader {
public:

	std::shared_ptr<Take> load(const std::string& filename);

private:

	std::shared_ptr<camera::Camera> load_camera(const rapidjson::Value& camera_value) const;

	film::Film* load_film(const rapidjson::Value& film_value) const;

	std::shared_ptr<rendering::Surface_integrator_factory> load_surface_integrator_factory(const rapidjson::Value& integrator_value) const;
};

}
