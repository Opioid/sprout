#include "controller_progressive.hpp"
#include "core/logging/logging.hpp"
#include "core/rendering/rendering_driver_progressive.hpp"
#include "core/scene/scene.hpp"
#include "core/take/take.hpp"
#include "core/progress/progress_sink.hpp"
#include "base/math/vector.inl"
#include "base/string/string.inl"
#include "base/thread/thread_pool.hpp"
#include <iostream>

namespace controller {

class Progressor_absolute : public progress::Sink {
public:

	Progressor_absolute() : iterations_(0) {}

	virtual void start(uint32_t /*resolution*/) final override {}

	virtual void end() final override {}

	virtual void tick() final override {
		++iterations_;
	}

	uint32_t iterations() const {
		return iterations_;
	}

private:

	uint32_t iterations_;
};

void progressive(const take::Take& take, scene::Scene& scene, thread::Pool& thread_pool) {
	logging::info("Progressive mode... type stuff to interact");

	if (!take.view.camera) {
		return;
	}

	rendering::Driver_progressive driver(take.surface_integrator_factory,
										 take.volume_integrator_factory,
										 take.sampler, scene, take.view,
										 thread_pool);

	Progressor_absolute progressor;

	driver.render(*take.exporter, progressor);

	for (;;) {
		std::string input;
		std::cin >> input;

		if ("abort" == input || "exit" == input || "quit" == input) {
			break;
		} else if ("iterations" == input) {
			logging::info(string::to_string(progressor.iterations()));
		}
	}

	driver.abort();
}

}
