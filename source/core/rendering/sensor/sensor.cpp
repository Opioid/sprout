#include "sensor.hpp"
#include "base/math/vector4.inl"
#include "base/thread/thread_pool.hpp"
#include "image/typed_image.hpp"

namespace rendering::sensor {

Sensor::Sensor(int32_t filter_radius)
    : dimensions_(0), filter_radius_(filter_radius), num_layers_(0) {}

Sensor::~Sensor() = default;

int2 Sensor::dimensions() const {
    return dimensions_;
}

void Sensor::resolve(Threads& threads, image::Float4& target) const {
    threads.run_range([this, &target](uint32_t /*id*/, int32_t begin,
                                      int32_t end) noexcept { resolve(begin, end, target); },
                      0, target.description().area());
}

void Sensor::resolve_accumulate(Threads& threads, image::Float4& target) const {
    threads.run_range(
        [this, &target](uint32_t /*id*/, int32_t begin, int32_t end) noexcept {
            resolve_accumulate(begin, end, target);
        },
        0, target.description().area());
}

void Sensor::resolve(aov::Property aov, Threads& threads, image::Float4& target) const {
    threads.run_range([this, aov, &target](uint32_t /*id*/, int32_t begin,
                                      int32_t end) noexcept { resolve(begin, end, aov, target); },
                      0, target.description().area());
}

void Sensor::resize(int2 dimensions, int32_t num_layers) {
    on_resize(dimensions, num_layers);

    aov_.resize(dimensions);

    dimensions_ = dimensions;

    num_layers_ = num_layers;
}

int32_t Sensor::filter_radius_int() const {
    return filter_radius_;
}

int4 Sensor::isolated_tile(int4 const& tile) const {
    int32_t const r = filter_radius_;
    return tile + int4(r, r, -r, -r);
}

void Sensor::clear(float weight) {
    on_clear(weight);

    aov_.clear();
}

void Sensor::add_pixel(int2 pixel, float4 const& value, float weight, aov::Property aov) {
    auto const d = dimensions();

    int32_t const id = d[0] * pixel[1] + pixel[0];

    aov_.add_pixel(id, value, weight, aov);
}

void Sensor::add_pixel_atomic(int2 pixel, float4 const& value, float weight, aov::Property aov) {
    auto const d = dimensions();

    int32_t const id = d[0] * pixel[1] + pixel[0];

    aov_.add_pixel(id, value, weight, aov);
}

void Sensor::resolve(int32_t begin, int32_t end, aov::Property aov, image::Float4& target) const {

}

}  // namespace rendering::sensor
