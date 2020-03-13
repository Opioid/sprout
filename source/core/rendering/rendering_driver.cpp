#include "rendering_driver.hpp"
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
#include "scene/scene.inl"
#include "take/take.hpp"

namespace rendering {

static uint32_t constexpr Num_particles_per_chunk = 1024;

Driver::Driver(thread::Pool& threads, uint32_t max_sample_size, progress::Sink& progressor)
    : threads_(threads),
      scene_(nullptr),
      view_(nullptr),
      workers_(memory::construct_aligned<Camera_worker>(threads.num_threads(), max_sample_size)),
      frame_(0),
      frame_view_(0),
      frame_iteration_(0),
      photon_infos_(new Photon_info[threads.num_threads()]),
      progressor_(progressor) {}

Driver::~Driver() {
    delete[] photon_infos_;

    memory::destroy_aligned(workers_, threads_.num_threads());
}

void Driver::init(take::View& view, Scene& scene, bool progressive) {
    view_ = &view;

    scene_ = &scene;

    Camera const& camera = *view.camera;

    tiles_.init(camera.crop(), 32, camera.sensor().filter_radius_int());

    int2 const d = camera.sensor_dimensions();

    camera.sensor().resize(d, progressive && view.lighttracers ? 2 : 1);

    target_.resize(d);

    int2 const r = camera.resolution();

    uint64_t const num_particles = uint64_t(r[0] * r[1]) *
                                   uint64_t(progressive ? 1 : view.num_particles_per_pixel);

#ifdef PARTICLE_GUIDING
    if (progressive) {
        ranges_.init(view.lighttracers ? num_particles : 0, 0, Num_particles_per_chunk);
    } else {
        uint64_t const head = num_particles / 10;
        uint64_t const tail = num_particles - head;

        ranges_.init(view.lighttracers ? head : 0, view.lighttracers ? tail : 0,
                     Num_particles_per_chunk);
    }
#else
    ranges_.init(view.lighttracers ? num_particles : 0, 0, Num_particles_per_chunk);
#endif

    integrator::particle::photon::Map* photon_map = nullptr;

    uint32_t const num_photons = view.photon_settings.num_photons;
    if (num_photons) {
        uint32_t const num_workers = threads_.num_threads();

        photon_map_.init(num_workers, view.photon_settings.num_photons,
                         view.photon_settings.search_radius, view.photon_settings.merge_radius);

        uint32_t range = num_photons / num_workers;
        if (num_photons % num_workers) {
            ++range;
        }

        photon_map = &photon_map_;
    }

    if (num_photons > 0 || view.lighttracers) {
        particle_importance_.init(scene);
    }

    for (uint32_t i = 0, len = threads_.num_threads(); i < len; ++i) {
        workers_[i].init(i, scene, camera, view.num_samples_per_pixel, view.surface_integrators,
                         *view.volume_integrators, *view.samplers, photon_map, view.photon_settings,
                         view.lighttracers, Num_particles_per_chunk, &particle_importance_);
    }
}

scene::camera::Camera& Driver::camera() {
    return *view_->camera;
}

scene::Scene const& Driver::scene() const {
    return *scene_;
}

scene::Scene& Driver::scene() {
    return *scene_;
}

image::Float4 const& Driver::target() const {
    return target_;
}

void Driver::render(Exporters& exporters) {
    for (uint32_t f = 0; f < view_->num_frames; ++f) {
        uint32_t const frame = view_->start_frame + f;

        render(frame);

        export_frame(frame, exporters);
    }
}

void Driver::render(uint32_t frame) {
    auto& camera = *view_->camera;

    scene_->finish();

    logging::info("Frame " + string::to_string(frame));

    auto const render_start = std::chrono::high_resolution_clock::now();

    uint64_t const start = frame * camera.frame_step();
    scene_->simulate(start, start + camera.frame_duration(), threads_);

    camera.update(*scene_, start, workers_[0]);

    particle_importance_.set_eye_position(
        scene_->prop_world_transformation(camera.entity()).position);

    auto const preparation_duration = chrono::seconds_since(render_start);
    logging::info("Preparation time %f s", preparation_duration);

    bake_photons(frame);

    render_frame_backward(frame);

    render_frame_forward(frame);

    auto const render_duration = chrono::seconds_since(render_start);
    logging::info("Render time %f s", render_duration);

    auto const pp_start = std::chrono::high_resolution_clock::now();

    if (int4(int2(0), camera.resolution()) != camera.crop()) {
        camera.sensor().fix_zero_weights();
    }

    if (ranges_.size() > 0 && view_->num_samples_per_pixel > 0) {
        view_->pipeline.apply_accumulate(camera.sensor(), target_, threads_);
    } else {
        view_->pipeline.apply(camera.sensor(), target_, threads_);
    }

    auto const pp_duration = chrono::seconds_since(pp_start);
    logging::info("Post-process time %f s", pp_duration);
}

void Driver::start_frame(uint32_t frame) {
    auto& camera = *view_->camera;

    scene_->finish();

    uint64_t const start = frame * camera.frame_step();
    scene_->simulate(start, start + camera.frame_duration(), threads_);

    camera.update(*scene_, start, workers_[0]);

    camera.sensor().clear(0.f);

    particle_importance_.set_training(false);
}

void Driver::render(uint32_t frame, uint32_t iteration) {
    render_frame_backward(frame, iteration);

    render_frame_forward(frame, iteration);

    auto& camera = *view_->camera;

    if (ranges_.size() > 0 && view_->num_samples_per_pixel > 0) {
        view_->pipeline.apply_accumulate(camera.sensor(), target_, threads_);
    } else {
        view_->pipeline.apply(camera.sensor(), target_, threads_);
    }
}

void Driver::export_frame(uint32_t frame, Exporters& exporters) const {
    auto const export_start = std::chrono::high_resolution_clock::now();

    for (auto& e : exporters) {
        e->write(target_, frame, threads_);
    }

    auto const export_duration = chrono::seconds_since(export_start);
    logging::info("Export time %f s", export_duration);
}

void Driver::render_frame_backward(uint32_t frame) {
    if (0 == ranges_.size()) {
        return;
    }

    logging::info("Tracing light rays...");

    auto const start = std::chrono::high_resolution_clock::now();

    frame_ = frame;

    auto& camera = *view_->camera;

    progressor_.start(ranges_.size());

    camera.sensor().clear(float(view_->num_particles_per_pixel));

#ifdef PARTICLE_GUIDING
    particle_importance_.set_training(true);
#else
    particle_importance_.set_training(false);
#endif

    ranges_.restart(0);

    threads_.run_parallel([this](uint32_t index) noexcept {
        auto& worker = workers_[index];

        for (ulong2 range; ranges_.pop(range);) {
            worker.particles(frame_, 0, range);

            progressor_.tick();
        }
    });

#ifdef PARTICLE_GUIDING

    particle_importance_.prepare_sampling(threads_);
    particle_importance_.set_training(false);

    ranges_.restart(1);

    threads_.run_parallel([this](uint32_t index) noexcept {
        auto& worker = workers_[index];

        for (ulong2 range; ranges_.pop(range);) {
            worker.particles(frame_, 0, range);

            progressor_.tick();
        }
    });

#endif

    // If there will be a forward pass later...
    if (view_->num_samples_per_pixel > 0) {
        view_->pipeline.seed(camera.sensor(), target_, threads_);
    }

    auto const duration = chrono::seconds_since(start);
    logging::info("Light ray time " + string::to_string(duration) + " s");
}

void Driver::render_frame_backward(uint32_t frame, uint32_t iteration) {
    if (0 == ranges_.size()) {
        return;
    }

    frame_ = frame;

    frame_iteration_ = iteration;

    auto& camera = *view_->camera;

    bool const forward = view_->num_samples_per_pixel > 0;

    if (forward) {
        camera.sensor().set_layer(1);
    }

    // This weight works because we assume 1 particle per pixel in progressive mode
    camera.sensor().set_weights(float(iteration + 1));

    ranges_.restart(0);

    threads_.run_parallel([this](uint32_t index) noexcept {
        auto& worker = workers_[index];

        uint64_t const offset = uint64_t(frame_iteration_) * ranges_.total();

        for (ulong2 range; ranges_.pop(range);) {
            worker.particles(frame_, offset, range);
        }
    });

    // If there will be a forward pass later...
    if (forward) {
        view_->pipeline.seed(camera.sensor(), target_, threads_);

        camera.sensor().set_layer(0);
    }
}

void Driver::render_frame_forward(uint32_t frame) {
    if (0 == view_->num_samples_per_pixel) {
        return;
    }

    logging::info("Tracing camera rays...");

    auto const start = std::chrono::high_resolution_clock::now();

    auto& camera = *view_->camera;

    camera.sensor().clear(0.f);

    frame_ = frame;

    progressor_.start(tiles_.size() * camera.num_views());

    for (uint32_t v = 0, len = camera.num_views(); v < len; ++v) {
        frame_view_ = v;

        tiles_.restart();

        threads_.run_parallel([this](uint32_t index) noexcept {
            auto& worker = workers_[index];

            uint32_t const num_samples = view_->num_samples_per_pixel;

            for (int4 tile; tiles_.pop(tile);) {
                worker.render(frame_, frame_view_, 0, tile, num_samples);

                progressor_.tick();
            }
        });
    }

    auto const duration = chrono::seconds_since(start);
    logging::info("Camera ray time " + string::to_string(duration) + " s");
}

void Driver::render_frame_forward(uint32_t frame, uint32_t iteration) {
    if (0 == view_->num_samples_per_pixel) {
        return;
    }

    auto& camera = *view_->camera;

    frame_ = frame;

    frame_iteration_ = iteration;

    for (uint32_t v = 0, len = camera.num_views(); v < len; ++v) {
        frame_view_ = v;

        tiles_.restart();

        threads_.run_parallel([this](uint32_t index) noexcept {
            auto& worker = workers_[index];

            uint32_t const num_samples = view_->num_samples_per_pixel;

            for (int4 tile; tiles_.pop(tile);) {
                worker.render(frame_, frame_view_, frame_iteration_, tile, num_samples);
            }
        });
    }
}

void Driver::bake_photons(uint32_t frame) {
    uint32_t const settings_num_photons = view_->photon_settings.num_photons;

    if (/*photons_baked_ || */ 0 == settings_num_photons) {
        return;
    }

    logging::info("Baking photons...");

    auto const start = std::chrono::high_resolution_clock::now();

    uint64_t num_paths = 0;
    uint32_t begin     = 0;

    float const iteration_threshold = view_->photon_settings.iteration_threshold;

    photon_map_.start();

#ifdef PHOTON_GUIDING
    uint32_t num_photons = std::max(settings_num_photons / 10, 1u);
    particle_importance_.set_training(true);
#else
    uint32_t num_photons = photon_settings_.num_photons;
    particle_importance_.set_training(false);
#endif

    frame_ = frame;

    for (uint32_t iteration = 0;; ++iteration) {
        frame_iteration_ = iteration;

        threads_.run_range(
            [this](uint32_t id, int32_t begin, int32_t end) {
                auto& worker = workers_[id];

                photon_infos_[id].num_paths = worker.bake_photons(begin, end, frame_,
                                                                  frame_iteration_);
            },
            static_cast<int32_t>(begin), static_cast<int32_t>(num_photons));

        for (uint32_t i = 0, len = threads_.num_threads(); i < len; ++i) {
            num_paths += uint64_t(photon_infos_[i].num_paths);
        }

        if (!num_paths) {
            logging::info("No photons");
            break;
        }

        uint32_t const new_begin = photon_map_.compile_iteration(num_photons, num_paths, threads_);

#ifdef PHOTON_GUIDING
        particle_importance_.prepare_sampling(threads_);
        particle_importance_.set_training(false);
#endif

        if (0 == new_begin || settings_num_photons == new_begin || 1.f <= iteration_threshold ||
            float(begin) / float(new_begin) > (1.f - iteration_threshold)) {
            break;
        }

        begin = new_begin;

        num_photons = settings_num_photons;
    }

    photon_map_.compile_finalize();

    auto const duration = chrono::seconds_since(start);
    logging::info("Photon time " + string::to_string(duration) + " s");
}

}  // namespace rendering
