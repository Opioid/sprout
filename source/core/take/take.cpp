#include "take.hpp"
#include "base/memory/array.inl"
#include "exporting/exporting_sink.hpp"
#include "rendering/integrator/particle/lighttracer.hpp"
#include "rendering/integrator/surface/surface_integrator.hpp"
#include "rendering/integrator/volume/volume_integrator.hpp"
#include "sampler/sampler.hpp"
#include "scene/camera/camera.hpp"

namespace take {

View::View() noexcept = default;

View::~View() noexcept {
    clear();
}

void View::clear() noexcept {
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

void View::init(thread::Pool& threads) noexcept {
    if (camera) {
        pipeline.init(*camera, threads);
    }
}

bool View::valid() const noexcept {
    return (nullptr != surface_integrators || nullptr != lighttracers) && nullptr != samplers &&
           nullptr != camera;
}

Take::Take() noexcept = default;

Take::~Take() noexcept {
    clear();
}

void Take::clear() noexcept {
    for (auto e : exporters) {
        delete e;
    }

    exporters.clear();

    view.clear();
}

}  // namespace take
