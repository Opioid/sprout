#pragma once

#include <string>
#include <memory>

namespace rendering {

class Surface_integrator_factory;

}

namespace take {

struct Take;

class Loader {
public:

	std::shared_ptr<Take> load(const std::string& filename);

private:

	std::shared_ptr<rendering::Surface_integrator_factory> load_surface_integrator_factory();
};

}
