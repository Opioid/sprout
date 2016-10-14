#pragma once

#include "take_settings.hpp"
#include "take_view.hpp"
#include "base/math/random/generator.hpp"
#include <string>
#include <memory>

namespace exporting { class Sink; }

namespace scene { namespace animation { class Animation; } }

namespace rendering { namespace integrator {

namespace surface { class Factory; }
namespace volume  { class Factory; }

}}

namespace sampler { class Factory; }

namespace take {

struct Take {
	Take();

	Settings	settings;
	std::string	scene_filename;
	take::View	view;

	std::shared_ptr<scene::animation::Animation> camera_animation;

	std::shared_ptr<rendering::integrator::surface::Factory> surface_integrator_factory;
	std::shared_ptr<rendering::integrator::volume::Factory>  volume_integrator_factory;

	std::shared_ptr<sampler::Factory> sampler_factory;

	std::unique_ptr<exporting::Sink> exporter;
};

}
