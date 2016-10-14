#pragma once

#include "rendering_driver.hpp"
#include "exporting/exporting_sink_statistics.hpp"
#include <thread>

namespace scene { namespace camera { class Camera; } }

namespace exporting { class Sink; }

namespace progress { class Sink; }

namespace rendering {

class Driver_progressive : public Driver {

public:

	Driver_progressive(Surface_integrator_factory surface_integrator_factory,
					   Volume_integrator_factory volume_integrator_factory,
					   std::shared_ptr<sampler::Factory> sampler_factory,
					   scene::Scene& scene,
					   take::View& view,
					   thread::Pool& thread_pool);

	void render(exporting::Sink& exporter);

	void abort();

	void schedule_restart(bool recompile);
	void schedule_statistics();

	void set_force_statistics(bool force);

	uint32_t iteration() const;

private:

	void render_loop(exporting::Sink& exporter);

	void restart();

	std::thread render_thread_;

	uint32_t iteration_;

	uint32_t samples_per_iteration_;

	bool rendering_;

	struct Schedule {
		bool restart	= false;
		bool recompile  = false;
		bool statistics = false;
	};

	Schedule schedule_;

	bool force_statistics_;

	exporting::Statistics statistics_;
};

}
