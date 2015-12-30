#pragma once

#include "take_settings.hpp"
#include "rendering/rendering_context.hpp"
#include "base/math/random/generator.hpp"
#include <string>
#include <memory>

namespace exporting { class Sink; }

namespace scene { namespace animation {

class Animation;

}}

namespace rendering { class Surface_integrator_factory; }

namespace sampler {

class Sampler;

}

namespace take {

struct Take {
	Take();

	Settings											   settings;
	std::string											   scene;
	rendering::Context									   context;
	std::shared_ptr<scene::animation::Animation>           camera_animation;
	std::shared_ptr<rendering::Surface_integrator_factory> surface_integrator_factory;
	std::shared_ptr<sampler::Sampler>					   sampler;
	std::unique_ptr<exporting::Sink>					   exporter;
	math::random::Generator rng;
};

}
