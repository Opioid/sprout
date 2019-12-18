#include "baking_driver.hpp"
#include "baking_item.hpp"
#include "baking_stencil.hpp"
#include "baking_worker.hpp"
#include "base/math/vector4.inl"
#include "base/memory/align.hpp"
#include "base/random/generator.inl"
#include "base/thread/thread_pool.hpp"
#include "image/encoding/png/png_writer.hpp"
#include "image/typed_image.hpp"
#include "logging/logging.hpp"
#include "progress/progress_sink.hpp"
#include "sampler/sampler.hpp"
#include "scene/camera/camera.hpp"
#include "scene/prop/interface_stack.inl"
#include "scene/scene.hpp"
#include "take/take.hpp"
#include "take/take_view.hpp"

#include <fstream>
#include <string>

#include <iostream>

namespace baking {

static uint32_t constexpr Slice_width = 256;

// static uint32_t constexpr Num_items = 1 * 1024 * 1024;

#ifdef BAKE_IMAGE
static uint32_t constexpr Num_items = Slice_width * Slice_width;
#else
static uint32_t constexpr Num_items = 16 * 1024 * 1024;
#endif

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

        workers_[i].baking_init(items_, Slice_width);
    }
}

Driver::~Driver() noexcept {
    delete[] items_;
    delete[] workers_;
}

void Driver::render() noexcept {
    {
        std::ifstream item_stream("radbak_v0_" + std::to_string(Num_items) + ".raw",
                                  std::ios::binary);

        item_stream.read(reinterpret_cast<char*>(items_), Num_items * sizeof(Item));

        std::ofstream stream("radbak_stencil_v0_" + std::to_string(Num_items) + ".raw",
                             std::ios::binary);

        float const scale = 0.5f;

        for (uint32_t i = 0; i < Num_items; ++i) {
            auto const& item = items_[i];

            Stencil stencil(float3(item.pos), float3(item.wi), scale);

            stream.write(reinterpret_cast<char const*>(stencil.data_),
                         Stencil::Num_data * sizeof(float));
        }

        return;
    }

    scene_.simulate(0, 0, threads_);

    progressor_.start(Num_items / 100);

    Stencil stencil(float3(0.f), float3(0.f, 1.f, 0.f), 0.1f);

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

#ifdef BAKE_IMAGE
    export_image();
#else
    std::ofstream stream("radbak_v0_" + std::to_string(Num_items) + ".raw", std::ios::binary);

    stream.write(reinterpret_cast<char const*>(items_), Num_items * sizeof(Item));
#endif
}

void Driver::export_image() const noexcept {
    memory::Buffer<float> buffer(Num_items);

    for (uint32_t i = 0; i < Num_items; ++i) {
        buffer[i] = items_[i].radiance;
    }

    image::encoding::png::Writer::write("slice.png", buffer.data(), int2(Slice_width), 1.f, true);

    std::ofstream stream(
        "slice_" + std::to_string(Slice_width) + "x" + std::to_string(Slice_width) + ".raw",
        std::ios::binary);

    stream.write(reinterpret_cast<char const*>(buffer.data()), Num_items * sizeof(float));
}

}  // namespace baking
