#include "take.hpp"
#include "base/memory/array.inl"
#include "base/thread/thread_pool.hpp"
#include "exporting/exporting_sink.hpp"
#include "rendering/integrator/particle/lighttracer.hpp"
#include "rendering/integrator/surface/surface_integrator.hpp"
#include "rendering/integrator/volume/volume_integrator.hpp"
#include "sampler/sampler.hpp"
#include "scene/camera/camera.hpp"

namespace take {

View::View() = default;

View::~View() {
    clear();
}

void View::clear() {
    pipeline.clear();

    aovs.clear();

    delete camera;
    camera = nullptr;

    delete samplers;
    samplers = nullptr;

    delete lighttracers;
    lighttracers = nullptr;

    delete volume_integrators;
    volume_integrators = nullptr;

    delete surface_integrators;
    surface_integrators = nullptr;
}

void View::init(Threads& threads) {
    if (camera) {
        aovs.init(threads.num_threads());

        pipeline.init(*camera, threads);

        uint32_t const spp = num_samples_per_pixel > 0 ? num_samples_per_pixel
                                                       : num_particles_per_pixel;

        camera->set_sample_spacing(1.f / std::sqrt(float(spp)));
    }
}

bool View::valid() const {
    return (nullptr != surface_integrators || nullptr != lighttracers) && nullptr != samplers &&
           nullptr != camera;
}

uint32_t View::num_particle_samples_per_pixel() const {
    if (lighttracers) {
        return num_particles_per_pixel * lighttracers->num_samples();
    }

    return 0;
}

Take::Take() = default;

Take::~Take() {
    clear();
}

void Take::clear() {
    for (auto e : exporters) {
        delete e;
    }

    exporters.clear();

    view.clear();
}

}  // namespace take
