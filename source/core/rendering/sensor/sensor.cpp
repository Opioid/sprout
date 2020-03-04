#include "sensor.hpp"
#include "base/math/vector4.inl"
#include "base/thread/thread_pool.hpp"
#include "image/typed_image.hpp"

namespace rendering::sensor {

Sensor::Sensor()
    : dimensions_(0), num_layers_(0) {}

Sensor::~Sensor() = default;

int2 Sensor::dimensions() const {
    return dimensions_;
}

void Sensor::resolve(thread::Pool& threads, image::Float4& target) const {
    threads.run_range([this, &target](uint32_t /*id*/, int32_t begin,
                                      int32_t end) noexcept { resolve(begin, end, target); },
                      0, target.description().area());
}

void Sensor::resolve_accumulate(thread::Pool& threads, image::Float4& target) const {
    threads.run_range(
        [this, &target](uint32_t /*id*/, int32_t begin, int32_t end) noexcept {
            resolve_accumulate(begin, end, target);
        },
        0, target.description().area());
}

void Sensor::resize(int2 dimensions, int32_t num_layers) {
    on_resize(dimensions, num_layers);

    dimensions_ = dimensions;

    num_layers_ = num_layers;
}

}  // namespace rendering::sensor
