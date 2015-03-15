#pragma once

#include <memory>

namespace scene {

class Scene;

}

namespace rendering {

class Surface_integrator_factory;
struct Context;

class Renderer {
public:

	Renderer(std::shared_ptr<Surface_integrator_factory> surface_integrator_factory);

	void render(const scene::Scene& scene, const Context& context) const;

private:

	std::shared_ptr<Surface_integrator_factory> surface_integrator_factory_;
};

}
