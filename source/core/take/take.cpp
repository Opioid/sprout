#include "take.hpp"
#include "base/memory/array.inl"
#include "exporting/exporting_sink.hpp"
#include "rendering/integrator/particle/lighttracer.hpp"
#include "rendering/integrator/surface/surface_integrator.hpp"
#include "rendering/integrator/volume/volume_integrator.hpp"
#include "sampler/sampler.hpp"
#include "scene/camera/camera.hpp"

namespace take {

View::View() noexcept {}

View::~View() noexcept {
    delete camera;
}

void View::clear() noexcept {
    delete camera;

    camera = nullptr;

    pipeline.clear();
}

void View::init(thread::Pool& threads) noexcept {
    if (camera) {
        pipeline.init(*camera, threads);
    }
}

Take::Take() noexcept {}

Take::~Take() noexcept {
    clear();
}

void Take::clear() noexcept {
    for (auto e : exporters) {
        delete e;
    }

    exporters.clear();

    delete sampler_factory;

    delete lighttracer_factory;

    delete volume_integrator_factory;

    delete surface_integrator_factory;

    view.clear();
}

}  // namespace take
