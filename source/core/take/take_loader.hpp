#pragma once

#include "base/json/rapidjson_types.hpp"
#include <string>
#include <memory>

namespace math { namespace random {

struct Generator;

}}

namespace camera {

class Camera;

}

namespace rendering {

namespace film {

class Film;

}

namespace sampler {

class Sampler;

}

class Surface_integrator_factory;

}

namespace take {

struct Take;

class Loader {
public:

	std::shared_ptr<Take> load(const std::string& filename);

private:

	std::shared_ptr<camera::Camera> load_camera(const rapidjson::Value& camera_value) const;

	rendering::film::Film* load_film(const rapidjson::Value& film_value) const;

	std::shared_ptr<rendering::sampler::Sampler> load_sampler(const rapidjson::Value& sampler_value, math::random::Generator& rng) const;

	std::shared_ptr<rendering::Surface_integrator_factory> load_surface_integrator_factory(const rapidjson::Value& integrator_value) const;
};

}
