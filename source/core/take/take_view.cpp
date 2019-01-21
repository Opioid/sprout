#include "take_view.hpp"
#include "scene/camera/camera.hpp"

namespace take {

View::View() noexcept {}

View::~View() noexcept {
    delete camera;
}

void View::init(thread::Pool& pool) noexcept {
    if (camera) {
        pipeline.init(*camera, pool);
    }
}

}  // namespace take
