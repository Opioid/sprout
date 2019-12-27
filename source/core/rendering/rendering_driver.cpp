#include "rendering_driver.hpp"
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

#ifdef PARTICLE_TRAINING

static uint64_t head(uint64_t total) noexcept {
    return uint64_t(0.1f * float(total));
}

static uint64_t tail(uint64_t total) noexcept {
    return total - head(total);
}

#endif

Driver::Driver(take::Take& take, Scene& scene, thread::Pool& threads,
               uint32_t max_sample_size) noexcept
    : scene_(scene),
      view_(take.view),
      threads_(threads),
      workers_(
          memory::construct_array_aligned<Camera_worker>(threads.num_threads(), tiles_, ranges_)),
      tiles_(take.view.camera->resolution(), 32, take.view.camera->sensor().filter_radius_int()),
#ifdef PARTICLE_TRAINING
      ranges_(take.lighttracer_factory ? head(take.view.num_particles) : 0,
              take.lighttracer_factory ? tail(take.view.num_particles) : 0,
              Num_particles_per_chunk),
#else
      ranges_(take.lighttracer_factory ? take.view.num_particles : 0, 0, Num_particles_per_chunk),
#endif

      target_(Description(take.view.camera->sensor_dimensions())),
      photon_map_(take.view.photon_settings.num_photons, take.view.photon_settings.search_radius,
                  take.view.photon_settings.merge_radius),
      photon_infos_(nullptr) {
    uint32_t const num_photons = take.view.photon_settings.num_photons;
    if (num_photons) {
        uint32_t const num_workers = threads.num_threads();

        photon_map_.init(num_workers);

        uint32_t range = num_photons / num_workers;
        if (num_photons % num_workers) {
            ++range;
        }

        photon_infos_ = new Photon_info[num_workers];
    }

    integrator::particle::photon::Map* photon_map = num_photons ? &photon_map_ : nullptr;

    if (num_photons > 0 || take.lighttracer_factory) {
        particle_importance_.init(scene);
    }

    for (uint32_t i = 0, len = threads.num_threads(); i < len; ++i) {
        workers_[i].init(i, scene, *take.view.camera, max_sample_size,
                         take.view.num_samples_per_pixel, *take.surface_integrator_factory,
                         *take.volume_integrator_factory, *take.sampler_factory, photon_map,
                         take.view.photon_settings, take.lighttracer_factory,
                         Num_particles_per_chunk, &particle_importance_);
    }
}

Driver::~Driver() noexcept {
    delete[] photon_infos_;
    memory::destroy_aligned(workers_, threads_.num_threads());
}

scene::camera::Camera& Driver::camera() noexcept {
    return *view_.camera;
}

scene::Scene const& Driver::scene() const noexcept {
    return scene_;
}

scene::Scene& Driver::scene() noexcept {
    return scene_;
}
}  // namespace rendering
