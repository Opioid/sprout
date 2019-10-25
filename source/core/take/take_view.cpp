#include "take_view.hpp"
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

}  // namespace take
