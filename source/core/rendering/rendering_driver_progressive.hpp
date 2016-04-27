#pragma once

#include "rendering_driver.hpp"
#include <thread>

namespace scene { namespace camera { class Camera; } }

namespace exporting { class Sink; }

namespace progress { class Sink; }

namespace rendering {

class Driver_progressive : public Driver {

public:

	Driver_progressive(Surface_integrator_factory surface_integrator_factory,
					   Volume_integrator_factory volume_integrator_factory,
					   std::shared_ptr<sampler::Sampler> sampler,
					   scene::Scene& scene, const take::View& view,
					   thread::Pool& thread_pool);

	void render(exporting::Sink& exporter, progress::Sink& progressor);

	void abort();

private:

	void render_loop(exporting::Sink& exporter, progress::Sink& progressor);

	std::thread render_thread_;

	bool rendering_;
};

}
