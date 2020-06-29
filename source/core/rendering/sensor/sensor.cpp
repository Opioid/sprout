#include "sensor.hpp"
#include "base/math/vector4.inl"
#include "base/thread/thread_pool.hpp"
#include "image/typed_image.hpp"

#include "image/encoding/png/png_writer.hpp"

namespace rendering::sensor {

Sensor::Sensor() : dimensions_(0), num_layers_(0), variance_(nullptr) {}

Sensor::~Sensor() {
    delete[] variance_;
}

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
    if (dimensions_[0] * dimensions_[1] != dimensions[0] * dimensions[1]) {
        delete variance_;

        variance_ = new float[dimensions[0] * dimensions[1]];
    }

    on_resize(dimensions, num_layers);

    dimensions_ = dimensions;

    num_layers_ = num_layers;
}

void Sensor::set_variance(int2 pixel, float variance) {
    if (pixel[0] < 0 || pixel[0] >= dimensions_[0] || pixel[1] < 0 || pixel[1] >= dimensions_[1]) {
        return;
    }

    int32_t const i = dimensions_[0] * pixel[1] + pixel[0];

    variance_[i] = variance;
}

void Sensor::export_variance() const {
    image::encoding::png::Writer::write_heatmap("variance.png", variance_, dimensions_);
}

}  // namespace rendering::sensor
