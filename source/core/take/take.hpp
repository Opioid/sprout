#pragma once

#include "rendering/context.hpp"
#include "base/math/random/generator.hpp"
#include <string>
#include <memory>

namespace sampler {

class Sampler;

}

namespace rendering {

class Surface_integrator_factory;

}

namespace take {

struct Take {
	Take();

	std::string											   scene;
	rendering::Context									   context;
	std::shared_ptr<rendering::Surface_integrator_factory> surface_integrator_factory;
	std::shared_ptr<sampler::Sampler>					   sampler;

	math::random::Generator rng;
};

}
