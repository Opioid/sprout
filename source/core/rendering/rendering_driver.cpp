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
#include "take/take_view.hpp"

namespace rendering {

using namespace image;

Driver::Driver(take::Take& take, Scene& scene, thread::Pool& thread_pool,
               uint32_t max_material_sample_size) noexcept
    : scene_(scene),
      view_(take.view),
      thread_pool_(thread_pool),
      workers_(memory::construct_array_aligned<Camera_worker>(thread_pool.num_threads(), tiles_)),
      tiles_(take.view.camera->resolution(), int2(32, 32),
             take.view.camera->sensor().filter_radius_int()),
      target_(Description(take.view.camera->sensor_dimensions())),
      lighttracing_(nullptr != take.lighttracer_factory),
      photon_settings_(take.photon_settings),
      photon_map_(take.photon_settings.num_photons, take.photon_settings.search_radius,
                  take.photon_settings.merge_radius, take.photon_settings.coarse_search_radius,
                  take.photon_settings.separate_indirect && take.photon_settings.indirect_photons),
      photon_infos_(nullptr) {
    uint32_t const num_photons = take.photon_settings.num_photons;
    if (num_photons) {
        uint32_t const num_workers = thread_pool.num_threads();

        photon_map_.init(scene, num_workers);

        uint32_t range = num_photons / num_workers;
        if (num_photons % num_workers) {
            ++range;
        }

        photon_infos_ = new Photon_info[num_workers];
    }

    integrator::particle::photon::Map* photon_map = num_photons ? &photon_map_ : nullptr;

    for (uint32_t i = 0, len = thread_pool.num_threads(); i < len; ++i) {
        workers_[i].init(i, take.settings, scene, *take.view.camera, max_material_sample_size,
                         take.view.num_samples_per_pixel, *take.surface_integrator_factory,
                         *take.volume_integrator_factory, *take.sampler_factory, photon_map,
                         take.photon_settings, take.lighttracer_factory);
    }
}

Driver::~Driver() noexcept {
    delete[] photon_infos_;
    memory::destroy_aligned(workers_, thread_pool_.num_threads());
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

size_t Driver::num_bytes() const noexcept {
    // Every worker must have exactly the same size, so we only need to query a single one
    size_t const worker_num_bytes = thread_pool_.num_threads() * workers_[0].num_bytes();

    return sizeof(*this) + worker_num_bytes + target_.num_bytes() + photon_map_.num_bytes();
}

}  // namespace rendering
