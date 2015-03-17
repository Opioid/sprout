#pragma once

#include "rendering/context.hpp"
#include "base/math/random/generator.hpp"
#include <string>
#include <memory>

namespace rendering {

class Surface_integrator_factory;

namespace sampler {

class Sampler;

}

}

namespace take {

struct Take {
	Take();

	std::string											   scene;
	rendering::Context									   context;
	std::shared_ptr<rendering::Surface_integrator_factory> surface_integrator_factory;
	std::shared_ptr<rendering::sampler::Sampler>		   sampler;

	math::random::Generator rng;
};

}
