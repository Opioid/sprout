#include "baking_driver.hpp"
#include "baking_item.hpp"
#include "baking_worker.hpp"
#include "base/math/vector4.inl"
#include "base/memory/align.hpp"
#include "base/random/generator.inl"
#include "base/thread/thread_pool.hpp"
#include "image/typed_image.hpp"
#include "logging/logging.hpp"
#include "progress/progress_sink.hpp"
#include "sampler/sampler.hpp"
#include "scene/camera/camera.hpp"
#include "scene/prop/interface_stack.inl"
#include "scene/scene.hpp"
#include "take/take.hpp"
#include "take/take_view.hpp"

#include "base/math/print.hpp"

#include <fstream>
#include <string>

namespace baking {

// static uint32_t constexpr Num_items = 1 * 1024 * 1024;

static uint32_t constexpr Num_items = 100 * 1024;

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
        workers_[i].init(i, scene, *take.view.camera, max_sample_size,
                         take.view.num_samples_per_pixel, *take.surface_integrator_factory,
                         *take.volume_integrator_factory, *take.sampler_factory, nullptr,
                         take.view.photon_settings, take.lighttracer_factory, 0, nullptr);

        workers_[i].baking_init(items_);
    }
}

Driver::~Driver() noexcept {
    delete[] items_;
    delete[] workers_;
}

void Driver::render() noexcept {
    scene_.simulate(0, 0, threads_);

    progressor_.start(Num_items / 100);

    threads_.run_range(
        [this](uint32_t id, int32_t begin, int32_t end) {
            auto& worker = workers_[id];

            for (int32_t b = begin; b < end;) {
                int32_t const e = b + 100;

                worker.bake(b, std::min(e, end));

                b = e;

                progressor_.tick();
            }
        },
        0, int32_t(num_items_));

    logging::info("Exporting...");

    std::ofstream stream("radbak_v0_" + std::to_string(Num_items) + ".raw", std::ios::binary);

    stream.write(reinterpret_cast<char const*>(items_), Num_items * sizeof(Item));

    /*
    std::ofstream txtstream("radbak.txt");

    for (uint32_t i = 0; i < Num_items; ++i) {
        txtstream << items_[i].pos << items_[i].wi << items_[i].radiance << "\n";
    }
    */
}

}  // namespace baking
