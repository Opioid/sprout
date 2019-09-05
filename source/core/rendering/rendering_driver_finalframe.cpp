#include "rendering_driver_finalframe.hpp"
#include "base/chrono/chrono.hpp"
#include "base/math/vector4.inl"
#include "base/memory/array.inl"
#include "base/string/string.hpp"
#include "base/thread/thread_pool.hpp"
#include "exporting/exporting_sink.hpp"
#include "image/texture/texture_adapter.hpp"
#include "logging/logging.hpp"
#include "progress/progress_sink.hpp"
#include "rendering/rendering_camera_worker.hpp"
#include "rendering/sensor/sensor.hpp"
#include "sampler/sampler.hpp"
#include "scene/camera/camera.hpp"
#include "scene/entity/composed_transformation.hpp"
#include "scene/scene.hpp"
#include "take/take_view.hpp"

namespace rendering {

Driver_finalframe::Driver_finalframe(take::Take& take, Scene& scene, thread::Pool& thread_pool,
                                     uint32_t max_sample_size, progress::Sink& progressor) noexcept
    : Driver(take, scene, thread_pool, max_sample_size), progressor_(progressor) {}

void Driver_finalframe::render(Exporters& exporters) noexcept {
    photons_baked_ = false;

    auto& camera = *view_.camera;
    auto& sensor = camera.sensor();

    uint32_t const forward_progress_range  = tiles_.size() * camera.num_views();
    uint32_t const backward_progress_range = ranges_.size() * camera.num_views();

    for (uint32_t f = 0; f < view_.num_frames; ++f) {
        uint32_t const current_frame = view_.start_frame + f;
        logging::info("Frame " + string::to_string(current_frame));

        auto const render_start = std::chrono::high_resolution_clock::now();

        uint64_t const start = current_frame * camera.frame_step();
        scene_.simulate(start, start + camera.frame_duration(), thread_pool_);

        camera.update(scene_, start, workers_[0]);

        particle_importance_.set_eye_position(
            scene_.prop_world_transformation(camera.entity()).position);

        bake_photons(current_frame);

        progressor_.start(backward_progress_range);

        render_frame_backward(current_frame);

        progressor_.end();

        progressor_.start(forward_progress_range);

        render_frame_forward(current_frame);

        progressor_.end();

        auto const render_duration = chrono::seconds_since(render_start);
        logging::info("Render time " + string::to_string(render_duration) + " s");

        auto const export_start = std::chrono::high_resolution_clock::now();

        if (ranges_.size() > 0 && view_.num_samples_per_pixel > 0) {
            view_.pipeline.apply_accumulate(sensor, target_, thread_pool_);
        } else {
            view_.pipeline.apply(sensor, target_, thread_pool_);
        }

        for (auto& e : exporters) {
            e->write(target_, current_frame, thread_pool_);
        }

        auto const export_duration = chrono::seconds_since(export_start);
        logging::info("Export time " + string::to_string(export_duration) + " s");
    }
}

void Driver_finalframe::render_frame_backward(uint32_t frame) noexcept {
    if (0 == ranges_.size()) {
        return;
    }

    frame_ = frame;

    logging::info("Tracing light rays...");

    auto const start = std::chrono::high_resolution_clock::now();

    view_.camera->sensor().clear(1.f);

#ifdef PARTICLE_TRAINING
    particle_importance_.set_training(true);
#else
    particle_importance_.set_training(false);
#endif

    for (uint32_t v = 0, len = view_.camera->num_views(); v < len; ++v) {
        iteration_ = v;

        ranges_.restart();

        thread_pool_.run_parallel([this](uint32_t index) noexcept {
            auto& worker = workers_[index];

            for (ulong2 range; ranges_.pop(0, range);) {
                worker.particles(frame_, iteration_, 0, range);

                progressor_.tick();
            }
        });
    }

#ifdef PARTICLE_TRAINING

    particle_importance_.prepare_sampling(thread_pool_);
    particle_importance_.set_training(false);

    for (uint32_t v = 0, len = view_.camera->num_views(); v < len; ++v) {
        iteration_ = v;

        ranges_.restart();

        thread_pool_.run_parallel([this](uint32_t index) noexcept {
            auto& worker = workers_[index];

            for (ulong2 range; ranges_.pop(1, range);) {
                worker.particles(frame_, iteration_, 1, range);

                progressor_.tick();
            }
        });
    }

#endif

    // If there will be a forward pass later...
    if (view_.num_samples_per_pixel > 0) {
        view_.pipeline.seed(view_.camera->sensor(), target_, thread_pool_);
    }

    auto const duration = chrono::seconds_since(start);
    logging::info("Light ray time " + string::to_string(duration) + " s");

    particle_importance_.export_importances();
}

void Driver_finalframe::render_frame_forward(uint32_t frame) noexcept {
    if (0 == view_.num_samples_per_pixel) {
        return;
    }

    logging::info("Tracing camera rays...");

    auto const start = std::chrono::high_resolution_clock::now();

    view_.camera->sensor().clear(0.f);

    frame_ = frame;

    for (uint32_t v = 0, len = view_.camera->num_views(); v < len; ++v) {
        iteration_ = v;

        tiles_.restart();

        thread_pool_.run_parallel([this](uint32_t index) noexcept {
            auto& worker = workers_[index];

            uint32_t const num_samples = view_.num_samples_per_pixel;

            for (int4 tile; tiles_.pop(tile);) {
                worker.render(frame_, iteration_, tile, num_samples);

                progressor_.tick();
            }
        });
    }

    auto const duration = chrono::seconds_since(start);
    logging::info("Camera ray time " + string::to_string(duration) + " s");
}

void Driver_finalframe::bake_photons(uint32_t frame) noexcept {
    if (/*photons_baked_ || */ !photon_infos_) {
        return;
    }

    logging::info("Baking photons...");

    auto const start = std::chrono::high_resolution_clock::now();

    uint64_t num_paths = 0;
    uint32_t begin     = 0;

    float const iteration_threshold = photon_settings_.iteration_threshold;

    photon_map_.start();

    uint32_t num_photons = photon_settings_.num_photons;  // / 10 + 1;

    frame_ = frame;

    for (uint32_t iteration = 0;; ++iteration) {
        iteration_ = iteration;

        thread_pool_.run_range(
            [this](uint32_t id, int32_t begin, int32_t end) noexcept {
                auto& worker = workers_[id];

                photon_infos_[id].num_paths = worker.bake_photons(begin, end, frame_, iteration_);
            },
            static_cast<int32_t>(begin),
            static_cast<int32_t>(
                num_photons) /*static_cast<int32_t>(photon_settings_.num_photons)*/);

        for (uint32_t i = 0, len = thread_pool_.num_threads(); i < len; ++i) {
            num_paths += uint64_t(photon_infos_[i].num_paths);
        }

        if (!num_paths) {
            logging::info("No photons");
            break;
        }

        uint32_t const new_begin = photon_map_.compile_iteration(num_photons, num_paths,
                                                                 thread_pool_);

        particle_importance_.prepare_sampling(thread_pool_);

        if (0 == new_begin || photon_settings_.num_photons == new_begin ||
            1.f <= iteration_threshold ||
            static_cast<float>(begin) / static_cast<float>(new_begin) >
                (1.f - iteration_threshold)) {
            break;
        }

        begin = new_begin;

        num_photons = photon_settings_.num_photons;
    }

    photon_map_.compile_finalize();

    particle_importance_.export_importances();

    auto const duration = chrono::seconds_since(start);
    logging::info("Photon time " + string::to_string(duration) + " s");

    photons_baked_ = true;
}

}  // namespace rendering
