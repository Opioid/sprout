#pragma once

#include "rendering/context.hpp"
#include <string>
#include <memory>

namespace rendering {

class Surface_integrator_factory;

}

namespace take {

struct Take {
	std::string											   scene;
	rendering::Context									   context;
	std::shared_ptr<rendering::Surface_integrator_factory> surface_integrator_factory;
};

}
