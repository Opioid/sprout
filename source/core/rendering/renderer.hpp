#pragma once

#include <memory>

namespace scene {

class Scene;

}

namespace rendering {

class Surface_integrator_factory;
struct Context;

namespace sampler {

class Sampler;

}

class Renderer {
public:

	Renderer(std::shared_ptr<Surface_integrator_factory> surface_integrator_factory, std::shared_ptr<sampler::Sampler> sampler);

	void render(const scene::Scene& scene, const Context& context) const;

private:

	std::shared_ptr<Surface_integrator_factory> surface_integrator_factory_;
	std::shared_ptr<sampler::Sampler> sampler_;
};

}
