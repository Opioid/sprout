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

      photon_infos_(new Photon_info[threads.num_threads()]) {}

Driver::~Driver() noexcept {
    delete[] photon_infos_;

    memory::destroy_aligned(workers_, threads_.num_threads());
}

void Driver::init(take::Take& take, Scene& scene) noexcept {
    view_ = &take.view;

    scene_ = &scene;

    tiles_.init(take.view.camera->resolution(), 32, take.view.camera->sensor().filter_radius_int());

#ifdef PARTICLE_TRAINING
    uint64_t const head = uint64_t(0.1f * float(take.view.num_particles));
    uint64_t const tail = take.view.num_particles - head;

    ranges_.init(take.lighttracers ? head : 0, take.lighttracers ? tail : 0,
                 Num_particles_per_chunk);
#else
    ranges_.init(take.lighttracer_pool ? take.view.num_particles : 0, 0, Num_particles_per_chunk),
#endif

    target_.resize(take.view.camera->sensor_dimensions());

    integrator::particle::photon::Map* photon_map = nullptr;

    uint32_t const num_photons = take.view.photon_settings.num_photons;
    if (num_photons) {
        uint32_t const num_workers = threads_.num_threads();

        photon_map_.init(num_workers, take.view.photon_settings.num_photons,
                         take.view.photon_settings.search_radius,
                         take.view.photon_settings.merge_radius);

        uint32_t range = num_photons / num_workers;
        if (num_photons % num_workers) {
            ++range;
        }

        photon_map = &photon_map_;
    }

    if (num_photons > 0 || take.lighttracers) {
        particle_importance_.init(scene);
    }

    for (uint32_t i = 0, len = threads_.num_threads(); i < len; ++i) {
        workers_[i].init(i, scene, *take.view.camera, take.view.num_samples_per_pixel,
                         *take.surface_integrators, *take.volume_integrators, *take.samplers,
                         photon_map, take.view.photon_settings, take.lighttracers,
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
}  // namespace rendering
