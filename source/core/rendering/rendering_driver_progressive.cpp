#include "rendering_driver_progressive.hpp"
#include "exporting/exporting_sink.hpp"
#include "logging/logging.hpp"
#include "rendering/sensor/sensor.hpp"
#include "sampler/sampler.hpp"
#include "scene/scene.hpp"
#include "scene/camera/camera.hpp"
#include "take/take_view.hpp"
#include "base/math/vector4.inl"
#include "base/string/string.hpp"
#include "base/thread/thread_pool.hpp"

namespace rendering {

Driver_progressive::Driver_progressive(Surface_integrator_factory surface_integrator_factory,
									   Volume_integrator_factory volume_integrator_factory,
									   std::shared_ptr<sampler::Factory> sampler_factory,
									   scene::Scene& scene,
									   take::View& view,
									   thread::Pool& thread_pool) :
	Driver(surface_integrator_factory, volume_integrator_factory,
		   sampler_factory, scene, view, thread_pool),
	iteration_(0),
	samples_per_iteration_(1),
	rendering_(false),
	force_statistics_(false) {}

void Driver_progressive::render(exporting::Sink& exporter) {
	if (rendering_) {
		return;
	}

	scene_.tick(thread_pool_);

	restart();

	rendering_ = true;

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

void Driver_progressive::schedule_restart(bool recompile) {
	schedule_.restart = true;
	schedule_.recompile = recompile;
}

void Driver_progressive::schedule_statistics() {
	schedule_.statistics = true;
}

void Driver_progressive::set_force_statistics(bool force) {
	force_statistics_ = force;
}

uint32_t Driver_progressive::iteration() const {
	return iteration_;
}

void Driver_progressive::render_loop(exporting::Sink& exporter) {
	for (uint32_t v = 0, len = view_.camera->num_views(); v < len; ++v) {
		tiles_.restart();

		thread_pool_.run_parallel(
			[this, v](uint32_t index) {
				auto& worker = workers_[index];

				for (;;) {
					int4 tile;
					if (!tiles_.pop(tile)) {
						break;
					}

					uint32_t begin_sample = iteration_ * samples_per_iteration_;
					uint32_t end_sample   = begin_sample + samples_per_iteration_;
					worker.render(*view_.camera, v, tile, begin_sample, end_sample, 0.f, 1.f);
				}
			}
		);
	}

	view_.pipeline.apply(view_.camera->sensor(), target_, thread_pool_);
	exporter.write(target_, iteration_, thread_pool_);

	if (schedule_.statistics || force_statistics_) {
		statistics_.write(target_, iteration_, thread_pool_);
		schedule_.statistics = false;
	}

	if (schedule_.restart) {
		restart();
	}
}

void Driver_progressive::restart() {
	view_.camera->update(workers_[0]);

	view_.camera->sensor().clear();
	iteration_ = 0;

	if (schedule_.recompile) {
		scene_.compile(thread_pool_);
	}

	schedule_.restart = false;
}

}
