#include "rendering_driver_progressive.hpp"
#include "exporting/exporting_sink.hpp"
#include "logging/logging.hpp"
#include "rendering/sensor/sensor.hpp"
#include "sampler/sampler.hpp"
#include "scene/scene.hpp"
#include "scene/camera/camera.hpp"
#include "take/take_view.hpp"
#include "base/math/vector.inl"
#include "base/string/string.inl"
#include "base/thread/thread_pool.hpp"

namespace rendering {

Driver_progressive::Driver_progressive(Surface_integrator_factory surface_integrator_factory,
									   Volume_integrator_factory volume_integrator_factory,
									   std::shared_ptr<sampler::Sampler> sampler,
									   scene::Scene& scene, const take::View& view,
									   thread::Pool& thread_pool) :
	Driver(surface_integrator_factory, volume_integrator_factory,
		   sampler, scene, view, thread_pool),
	iteration_(0),
	rendering_(false) {}

void Driver_progressive::render(exporting::Sink& exporter) {
	if (rendering_) {
		return;
	}

	scene_.tick(thread_pool_);

	view_.camera->update_focus(workers_[0]);

	view_.camera->sensor().clear();

	iteration_ = 0;
	rendering_ = true;
	schedule_.restart = false;

	render_thread_ = std::thread([this, &exporter]() {
		for (; rendering_; ++iteration_) {
			render_loop(exporter);
		}
	});
}

void Driver_progressive::abort() {
	rendering_ = false;

	render_thread_.join();
}

void Driver_progressive::schedule_restart() {
	schedule_.restart = true;
}

uint32_t Driver_progressive::iteration() const {
	return iteration_;
}

void Driver_progressive::render_loop(exporting::Sink& exporter) {
	for (uint32_t v = 0, len = view_.camera->num_views(); v < len; ++v) {
		tiles_.restart();

		thread_pool_.run(
			[this, v](uint32_t index) {
				auto& worker = workers_[index];

				for (;;) {
					math::Recti tile;
					if (!tiles_.pop(tile)) {
						break;
					}

					worker.render(*view_.camera, v, tile, iteration_, iteration_ + 1, 0.f, 1.f);
				}
			}
		);
	}

	view_.camera->sensor().resolve(thread_pool_, target_);
	exporter.write(target_, iteration_, thread_pool_);

	if (schedule_.restart) {
		view_.camera->sensor().clear();
		iteration_ = 0;

		schedule_.restart = false;
	}
}

}
