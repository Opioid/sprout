#include "take.hpp"
#include "base/memory/array.inl"
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

    pipeline.clear();
}

void View::init(thread::Pool& threads) {
    if (camera) {
        pipeline.init(*camera, threads);

        camera->set_differential_scale(1.f / std::sqrt(num_samples_per_pixel));
    }
}

bool View::valid() const {
    return (nullptr != surface_integrators || nullptr != lighttracers) && nullptr != samplers &&
           nullptr != camera;
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
