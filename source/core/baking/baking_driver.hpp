#pragma once

#include <memory>

namespace take { struct View; }

namespace scene { class Scene; }

namespace sampler { class Factory; }

namespace thread { class Pool; }

namespace exporting { class Sink; }

namespace progress { class Sink; }

namespace rendering::integrator {

namespace surface { class Factory; }
namespace volume { class Factory; }

}

namespace baking {

class Driver {

public:

	using Surface_integrator_factory = rendering::integrator::surface::Factory;
	using Volume_integrator_factory  = rendering::integrator::volume::Factory;

	Driver(std::shared_ptr<Surface_integrator_factory> surface_integrator_factory,
		   std::shared_ptr<Volume_integrator_factory> volume_integrator_factory,
		   std::shared_ptr<sampler::Factory> sampler_factory);


	void render(scene::Scene& scene, const take::View& view,
				thread::Pool& thread_pool, uint32_t max_sample_size,
				exporting::Sink& exporter, progress::Sink& progressor);

private:

	std::shared_ptr<Surface_integrator_factory> surface_integrator_factory_;
	std::shared_ptr<Volume_integrator_factory> volume_integrator_factory_;
	std::shared_ptr<sampler::Factory> sampler_factory_;
};

}
