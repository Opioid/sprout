#include "baking_driver.hpp"
#include "baking_worker.hpp"
#include "base/math/vector4.inl"
#include "base/memory/align.hpp"
#include "base/random/generator.inl"
#include "base/thread/thread_pool.hpp"
#include "image/typed_image.hpp"
#include "sampler/sampler.hpp"
#include "scene/camera/camera.hpp"
#include "scene/prop/interface_stack.inl"
#include "scene/scene.hpp"
#include "take/take.hpp"
#include "take/take_view.hpp"

#include <fstream>

namespace baking {

Driver::Driver(take::Take& take, Scene& scene, thread::Pool& thread_pool,
               uint32_t max_material_sample_size, progress::Sink& progressor) noexcept
    : scene_(scene),
      view_(take.view),
      thread_pool_(thread_pool),
      workers_(new Baking_worker[thread_pool.num_threads()]),
      progressor_(progressor) {
    for (uint32_t i = 0, len = thread_pool.num_threads(); i < len; ++i) {
        workers_[i].init(i, take.settings, scene, *take.view.camera, max_material_sample_size,
                         take.view.num_samples_per_pixel, *take.surface_integrator_factory,
                         *take.volume_integrator_factory, *take.sampler_factory, nullptr,
                         take.photon_settings, take.lighttracer_factory, 0, nullptr);
    }
}

Driver::~Driver() noexcept {
    delete[] workers_;
}

void Driver::render() noexcept {}

}  // namespace baking
