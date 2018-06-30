#include "rendering_driver_finalframe.hpp"
#include "base/chrono/chrono.hpp"
#include "base/math/vector4.inl"
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
#include "scene/scene.hpp"
#include "take/take_view.hpp"

namespace rendering {

Driver_finalframe::Driver_finalframe(take::Take& take, scene::Scene& scene,
                                     thread::Pool& thread_pool, uint32_t max_sample_size)
    : Driver(take, scene, thread_pool, max_sample_size) {}

void Driver_finalframe::render(Exporters& exporters, progress::Sink& progressor) {
    photons_baked_ = false;

    auto& camera = *view_.camera;
    auto& sensor = camera.sensor();

    uint32_t const progress_range = calculate_progress_range(scene_, camera, tiles_.size(),
                                                             view_.num_samples_per_pixel);

    float const start_frame = static_cast<float>(view_.start_frame);
    float       tick_offset = scene_.seek(start_frame * camera.frame_duration(), thread_pool_);
    float       tick_rest   = scene_.tick_duration() - tick_offset;

    camera.update(scene_, workers_[0]);

    for (uint32_t f = 0; f < view_.num_frames; ++f) {
        uint32_t const current_frame = view_.start_frame + f;
        logging::info("Frame " + string::to_string(current_frame));

        auto const render_start = std::chrono::high_resolution_clock::now();

        sensor.clear();
        current_sample_ = 0;

        progressor.start(progress_range);

        if (0.f == camera.frame_duration()) {
            scene_.tick(thread_pool_);
            camera.update(scene_, workers_[0]);
            render_subframe(0.f, 0.f, 1.f, progressor);
        } else if (!camera.motion_blur()) {
            float frame_offset = 0.f;
            float frame_rest   = camera.frame_duration();

            bool rendered = false;

            while (frame_rest > 0.f) {
                if (tick_rest <= 0.f) {
                    scene_.tick(thread_pool_);
                    camera.update(scene_, workers_[0]);
                    tick_offset = 0.f;
                    tick_rest   = scene_.tick_duration();
                }

                float const subframe_slice = std::min(tick_rest, frame_rest);

                if (!rendered) {
                    float const normalized_tick_offset = tick_offset / scene_.tick_duration();

                    render_subframe(normalized_tick_offset, 0.f, 1.f, progressor);

                    rendered = true;
                }

                tick_offset += subframe_slice;
                tick_rest -= subframe_slice;

                frame_offset += subframe_slice;
                frame_rest -= subframe_slice;
            }
        } else {
            float frame_offset = 0.f;
            float frame_rest   = camera.frame_duration();

            while (frame_rest > 0.f) {
                if (tick_rest <= 0.f) {
                    scene_.tick(thread_pool_);
                    camera.update(scene_, workers_[0]);
                    tick_offset = 0.f;
                    tick_rest   = scene_.tick_duration();
                }

                float const subframe_slice = std::min(tick_rest, frame_rest);

                float const normalized_tick_offset = tick_offset / scene_.tick_duration();
                float const normalized_tick_slice  = subframe_slice / scene_.tick_duration();
                float const normalized_frame_slice = subframe_slice / camera.frame_duration();

                render_subframe(normalized_tick_offset, normalized_tick_slice,
                                normalized_frame_slice, progressor);

                tick_offset += subframe_slice;
                tick_rest -= subframe_slice;

                frame_offset += subframe_slice;
                frame_rest -= subframe_slice;
            }
        }

        progressor.end();

        auto const render_duration = chrono::seconds_since(render_start);
        logging::info("Render time " + string::to_string(render_duration) + " s");

        auto const export_start = std::chrono::high_resolution_clock::now();

        view_.pipeline.apply(sensor, target_, thread_pool_);

        for (auto& e : exporters) {
            e->write(target_, current_frame, thread_pool_);
        }

        auto const export_duration = chrono::seconds_since(export_start);
        logging::info("Export time " + string::to_string(export_duration) + " s");
    }
}

void Driver_finalframe::render_subframe(float normalized_tick_offset, float normalized_tick_slice,
                                        float normalized_frame_slice, progress::Sink& progressor) {
    bake_photons();

    float const num_samples       = static_cast<float>(view_.num_samples_per_pixel);
    float const samples_per_slice = normalized_frame_slice * num_samples;

    uint32_t const sample_begin = current_sample_;
    uint32_t const sample_range = std::max(static_cast<uint32_t>(samples_per_slice + 0.5f), 1u);
    uint32_t const sample_end = std::min(sample_begin + sample_range, view_.num_samples_per_pixel);

    if (sample_begin == sample_end) {
        return;
    }

    for (uint32_t v = 0, len = view_.camera->num_views(); v < len; ++v) {
        tiles_.restart();

        thread_pool_.run_parallel([this, v, sample_begin, sample_end, normalized_tick_offset,
                                   normalized_tick_slice, &progressor](uint32_t index) {
            auto& worker = workers_[index];

            for (int4 tile; tiles_.pop(tile);) {
                worker.render(*view_.camera, v, tile, sample_begin, sample_end,
                              normalized_tick_offset, normalized_tick_slice);

                progressor.tick();
            }
        });
    }

    current_sample_ = sample_end;
}

void Driver_finalframe::bake_photons() {
    if (photons_baked_ || !photon_infos_) {
        return;
    }

    logging::info("Baking photons...");

    auto const start = std::chrono::high_resolution_clock::now();

    thread_pool_.run_parallel([this](uint32_t index) {
        auto& worker = workers_[index];

        photon_infos_[index].num_paths = worker.bake_photons(photon_infos_[index].range);
    });

    uint32_t num_paths = 0;
    for (uint32_t i = 0, len = thread_pool_.num_threads(); i < len; ++i) {
        num_paths += photon_infos_[i].num_paths;
    }

    photon_map_.compile(num_paths, scene_.aabb(), thread_pool_);

    auto const duration = chrono::seconds_since(start);
    logging::info("Photon time " + string::to_string(duration) + " s");

    photons_baked_ = true;
}

uint32_t Driver_finalframe::calculate_progress_range(scene::Scene const&          scene,
                                                     const scene::camera::Camera& camera,
                                                     uint32_t                     num_tiles,
                                                     uint32_t num_samples_per_iteration) {
    float num_subframes = 1.f;

    if (camera.frame_duration() > 0.f && camera.motion_blur()) {
        num_subframes = std::min(camera.frame_duration() / scene.tick_duration(),
                                 static_cast<float>(num_samples_per_iteration));
    }

    float const range = static_cast<float>(num_tiles * camera.num_views()) * num_subframes;

    return static_cast<uint32_t>(range);
}

}  // namespace rendering
