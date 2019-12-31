#include "rendering_driver_progressive.hpp"
#include "base/math/vector4.inl"
#include "base/string/string.hpp"
#include "base/thread/thread_pool.hpp"
#include "exporting/exporting_sink.hpp"
#include "logging/logging.hpp"
#include "rendering/sensor/sensor.hpp"
#include "rendering_camera_worker.hpp"
#include "sampler/sampler.hpp"
#include "scene/camera/camera.hpp"
#include "scene/scene.hpp"
#include "take/take.hpp"

namespace rendering {

Driver_progressive::Driver_progressive(thread::Pool& threads, uint32_t max_sample_size) noexcept
    : Driver(threads, max_sample_size),
      iteration_(0),
      samples_per_iteration_(1),
      rendering_(false),
      force_statistics_(false) {}

void Driver_progressive::render(exporting::Sink& exporter) {
    if (rendering_) {
        return;
    }

    scene_->finish();

    restart();

    rendering_ = true;

    render_thread_ = std::thread([this, &exporter]() {
        for (; rendering_;) {
            if (render_loop(exporter)) {
                ++iteration_;
            }
        }
    });
}

void Driver_progressive::abort() {
    rendering_ = false;

    render_thread_.join();
}

void Driver_progressive::schedule_restart(bool recompile) {
    schedule_.restart   = true;
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

bool Driver_progressive::render_loop(exporting::Sink& exporter) {
    for (uint32_t v = 0, len = view_->camera->num_views(); v < len; ++v) {
        tiles_.restart();

        threads_.run_parallel([this, v](uint32_t index) {
            auto& worker = workers_[index];

            for (;;) {
                int4 tile;
                if (!tiles_.pop(tile)) {
                    break;
                }

                worker.render(0, v, tile, samples_per_iteration_);
            }
        });
    }

    view_->pipeline.apply(view_->camera->sensor(), target_, threads_);
    exporter.write(target_, iteration_, threads_);

    if (schedule_.statistics || force_statistics_) {
        statistics_.write(target_, iteration_, threads_);
        schedule_.statistics = false;
    }

    if (schedule_.restart) {
        restart();
        return false;
    }

    return true;
}

void Driver_progressive::restart() {
    if (schedule_.recompile) {
        scene_->compile(0, threads_);
    }

    schedule_.restart = false;

    view_->camera->update(*scene_, 0, workers_[0]);
    view_->camera->sensor().clear(0.f);
    iteration_ = 0;
}

}  // namespace rendering
