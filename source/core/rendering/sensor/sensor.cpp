#include "sensor.hpp"
#include "base/math/vector4.inl"
#include "base/thread/thread_pool.hpp"
#include "image/typed_image.hpp"

namespace rendering::sensor {

Sensor::Sensor(int2 dimensions, float exposure) noexcept
    : dimensions_(dimensions), exposure_factor_(std::exp2(exposure)) {}

Sensor::~Sensor() noexcept {}

int2 Sensor::dimensions() const noexcept {
    return dimensions_;
}

void Sensor::resolve(thread::Pool& pool, image::Float4& target) const noexcept {
    pool.run_range([ this, &target ](uint32_t /*id*/, int32_t begin, int32_t end) noexcept {
        resolve(begin, end, target);
    },
                   0, target.description().area());
}

void Sensor::resolve_accumulate(thread::Pool& pool, image::Float4& target) const noexcept {
    pool.run_range([ this, &target ](uint32_t /*id*/, int32_t begin, int32_t end) noexcept {
        resolve_accumulate(begin, end, target);
    },
                   0, target.description().area());
}

}  // namespace rendering::sensor
