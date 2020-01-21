#include "rendering_driver.hpp"
#include "base/math/aabb.inl"
#include "base/math/vector4.inl"
#include "base/memory/align.hpp"
#include "base/random/generator.inl"
#include "base/thread/thread_pool.hpp"
#include "image/typed_image.hpp"
#include "rendering/sensor/sensor.hpp"
#include "rendering_camera_worker.hpp"
#include "sampler/sampler.hpp"
#include "scene/camera/camera.hpp"
#include "scene/prop/interface_stack.inl"
#include "scene/scene.hpp"
#include "take/take.hpp"

namespace rendering {

using namespace image;

static uint32_t constexpr Num_particles_per_chunk = 1024;

Driver::Driver(thread::Pool& threads, uint32_t max_sample_size) noexcept
    : threads_(threads),
      workers_(memory::construct_array_aligned<Camera_worker>(threads.num_threads(),
                                                              max_sample_size, tiles_, ranges_)),
      frame_(0),
      frame_view_(0),
      frame_iteration_(0),
      photon_infos_(new Photon_info[threads.num_threads()]) {}

Driver::~Driver() noexcept {
    delete[] photon_infos_;

    memory::destroy_aligned(workers_, threads_.num_threads());
}

void Driver::init(take::View& view, Scene& scene, bool progressive) noexcept {
    view_ = &view;

    scene_ = &scene;

    tiles_.init(view.camera->resolution(), 32, view.camera->sensor().filter_radius_int());

#ifdef PARTICLE_TRAINING
    if (progressive) {
        ranges_.init(view.lighttracers ? view.num_particles : 0, 0, Num_particles_per_chunk);
    } else {
        uint64_t const head = uint64_t(0.1f * float(view.num_particles));
        uint64_t const tail = view.num_particles - head;

        ranges_.init(view.lighttracers ? head : 0, view.lighttracers ? tail : 0,
                     Num_particles_per_chunk);
    }
#else
    ranges_.init(view.lighttracers ? view.num_particles : 0, 0, Num_particles_per_chunk);
#endif

    target_.resize(view.camera->sensor_dimensions());

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
        workers_[i].init(i, scene, *view.camera, view.num_samples_per_pixel,
                         view.surface_integrators, *view.volume_integrators, *view.samplers,
                         photon_map, view.photon_settings, view.lighttracers,
                         Num_particles_per_chunk, &particle_importance_);
    }
}

scene::camera::Camera& Driver::camera() noexcept {
    return *view_->camera;
}

scene::Scene const& Driver::scene() const noexcept {
    return *scene_;
}

scene::Scene& Driver::scene() noexcept {
    return *scene_;
}

image::Float4 const& Driver::target() const noexcept {
    return target_;
}

}  // namespace rendering
