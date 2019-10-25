#include "baking_driver.hpp"
#include "baking_item.hpp"
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

static uint32_t constexpr Num_items = 1024 * 1024;

Driver::Driver(take::Take& take, Scene& scene, thread::Pool& threads, uint32_t max_sample_size,
               progress::Sink& progressor) noexcept
    : scene_(scene),
      view_(take.view),
      threads_(threads),
      workers_(new Baking_worker[threads.num_threads()]),
      progressor_(progressor),
      num_items_(Num_items),
      items_(new Item[Num_items]) {
    for (uint32_t i = 0, len = threads.num_threads(); i < len; ++i) {
        workers_[i].init(i, take.settings, scene, *take.view.camera, max_sample_size,
                         take.view.num_samples_per_pixel, *take.surface_integrator_factory,
                         *take.volume_integrator_factory, *take.sampler_factory, nullptr,
                         take.photon_settings, take.lighttracer_factory, 0, nullptr);
    }
}

Driver::~Driver() noexcept {
    delete[] items_;
    delete[] workers_;
}

void Driver::render() noexcept {
    scene_.simulate(0, scene_.tick_duration(), threads_);

    threads_.run_range(
        [](uint32_t id, int32_t begin, int32_t end) {

        },
        0, num_items_);
}

}  // namespace baking
