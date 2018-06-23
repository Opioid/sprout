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
#include "take/take_view.hpp"

namespace rendering {

Driver_progressive::Driver_progressive(take::Take& take, scene::Scene& scene,
                                       thread::Pool& thread_pool, uint32_t max_sample_size)
    : Driver(take, scene, thread_pool, max_sample_size),
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
    for (uint32_t v = 0, len = view_.camera->num_views(); v < len; ++v) {
        tiles_.restart();

        thread_pool_.run_parallel([this, v](uint32_t index) {
            auto& worker = workers_[index];

            for (;;) {
                int4 tile;
                if (!tiles_.pop(tile)) {
                    break;
                }

                uint32_t const begin_sample = iteration_ * samples_per_iteration_;
                uint32_t const end_sample   = begin_sample + samples_per_iteration_;
                worker.render(*view_.camera, v, tile, begin_sample, end_sample, 0.f, 1.f);
            }
        });
    }

    view_.pipeline.apply(view_.camera->sensor(), target_, thread_pool_);
    exporter.write(target_, iteration_, thread_pool_);

    if (schedule_.statistics || force_statistics_) {
        statistics_.write(target_, iteration_, thread_pool_);
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
        scene_.compile(thread_pool_);
    }

    schedule_.restart = false;

    view_.camera->update(scene_, workers_[0]);
    view_.camera->sensor().clear();
    iteration_ = 0;
}

}  // namespace rendering
