#pragma once

#include <memory>

namespace take { struct View; }

namespace scene { class Scene; }

namespace sampler { class Sampler; }

namespace thread { class Pool; }

namespace exporting { class Sink; }

namespace progress { class Sink; }

namespace rendering {

namespace integrator {

namespace surface { class Integrator_factory; }
namespace volume { class Integrator_factory; }

}}

namespace baking {

class Driver {
public:

	Driver(std::shared_ptr<rendering::integrator::surface::Integrator_factory> surface_integrator_factory,
		   std::shared_ptr<rendering::integrator::volume::Integrator_factory> volume_integrator_factory,
		   std::shared_ptr<sampler::Sampler> sampler);


	void render(scene::Scene& scene, const take::View& view, thread::Pool& thread_pool,
				exporting::Sink& exporter, progress::Sink& progressor);

private:

	std::shared_ptr<rendering::integrator::surface::Integrator_factory> surface_integrator_factory_;
	std::shared_ptr<rendering::integrator::volume::Integrator_factory> volume_integrator_factory_;
	std::shared_ptr<sampler::Sampler> sampler_;
};

}
