#include "sensor.hpp"
#include "base/math/vector4.inl"
#include "base/thread/thread_pool.hpp"
#include "image/typed_image.hpp"
#include "memory/buffer.hpp"

// #include "image/encoding/png/png_writer.hpp"

namespace rendering::sensor {

Sensor::Sensor(int32_t filter_radius, bool adaptive)
    : dimensions_(0),
      filter_radius_(filter_radius),
      adaptive_(adaptive),
      num_layers_(0),
      variance_(nullptr) {}

Sensor::~Sensor() {
    delete[] variance_;
}

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

void Sensor::resolve(uint32_t slot, AOV property, Threads& threads, image::Float4& target) const {
    threads.run_range(
        [this, slot, property, &target](uint32_t /*id*/, int32_t begin, int32_t end) noexcept {
            resolve(begin, end, slot, property, target);
        },
        0, target.description().area());
}

void Sensor::resize(int2 dimensions, int32_t num_layers, aov::Value_pool const& aovs) {
    if (adaptive_) {
        if (dimensions_[0] * dimensions_[1] != dimensions[0] * dimensions[1] || !variance_) {
            delete[] variance_;

            int32_t const m = filter_radius_int() > 0 ? 2 : 1;

            variance_ = new float[m * dimensions[0] * dimensions[1]];
        }
    } else if (variance_) {
        delete[] variance_;
    }

    on_resize(dimensions, num_layers);

    aov_.resize(dimensions, aovs);

    dimensions_ = dimensions;

    num_layers_ = num_layers;
}

bool Sensor::adaptive() const {
    return adaptive_;
}

uint32_t Sensor::num_samples_to_estimate(uint32_t max_samples) {
    return std::max(uint32_t(std::ceil(0.1f * float(max_samples))), 16u);
}

uint32_t Sensor::num_samples_by_estimate(int2 pixel, uint32_t max_samples) const {
    pixel = clamp(pixel, 0, dimensions_ - 1);

    int32_t const id = dimensions_[0] * pixel[1] + pixel[0];

    int32_t const i = variance_start_ + id;

    float const nv = variance_[i] / max_variance_;

    float const f = nv * nv;

    return std::min(uint32_t(std::ceil(f * float(max_samples))), max_samples);
}

void Sensor::set_variance_estimate(int2 pixel, float variance) {
    if (pixel[0] < 0 || pixel[0] >= dimensions_[0] || pixel[1] < 0 || pixel[1] >= dimensions_[1]) {
        return;
    }

    int32_t const i = dimensions_[0] * pixel[1] + pixel[0];

    variance_[i] = variance;
}

void Sensor::normalize_variance_estimate(Threads& threads) {
    int32_t const r = filter_radius_int();

    int2 const d = dimensions_;

    int32_t const area = d[0] * d[1];

    float max_variance = 0.f;

    memory::Buffer<float> max_variances(threads.num_threads(), 0.f);

    bool const filtered = r > 0;

    if (filtered) {
        float const* source = variance_;

        float* destination = variance_ + area;

        threads.run_range(
            [source, destination, d, r, &max_variances](uint32_t id, int32_t begin,
                                                        int32_t end) noexcept {
                float max_variance = 0.f;

                for (int32_t y = begin; y < end; ++y) {
                    for (int32_t x = 0; x < d[0]; ++x) {
                        float variance = 0.f;
                        for (int32_t fy = -r; fy <= r; ++fy) {
                            for (int32_t fx = -r; fx <= r; ++fx) {
                                int32_t const lx = x + fx;
                                int32_t const ly = y + fy;

                                if (lx < 0 || lx >= d[0] || ly < 0 || ly >= d[1]) {
                                    continue;
                                }

                                int32_t const i = d[0] * ly + lx;

                                variance = std::max(variance, source[i]);
                            }
                        }

                        int32_t const i = d[0] * y + x;

                        destination[i] = variance;

                        max_variance = std::max(max_variance, variance);
                    }
                }

                max_variances[id] = max_variance;
            },
            0, d[1]);

    } else {
        float const* source = variance_;

        threads.run_range(
            [source, d, &max_variances](uint32_t id, int32_t begin, int32_t end) noexcept {
                float max_variance = 0.f;

                for (int32_t y = begin; y < end; ++y) {
                    for (int32_t x = 0; x < d[0]; ++x) {
                        int32_t const i = d[0] * y + x;

                        float const variance = source[i];

                        max_variance = std::max(max_variance, variance);
                    }
                }

                max_variances[id] = max_variance;
            },
            0, d[1]);
    }

    for (uint32_t i = 0, len = threads.num_threads(); i < len; ++i) {
        max_variance = std::max(max_variance, max_variances[i]);
    }

    variance_start_ = filtered ? area : 0;
    max_variance_   = max_variance;

//    float const* source = variance_ + (filtered ? area : 0);

//    float* destination = variance_;

//    threads.run_range(
//        [source, destination, max_variance](uint32_t /*id*/, int32_t begin, int32_t end) noexcept {
//            for (int32_t i = begin; i < end; ++i) {
//                float const nv = source[i] / max_variance;
//                destination[i] = nv * nv;
//            }
//        },
//        0, area);

//    image::encoding::png::Writer::write_heatmap("variance.png", variance_, dimensions_, threads);
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

void Sensor::add_AOV(int2 pixel, uint32_t slot, float3 const& value, float weight) {
    auto const d = dimensions();

    int32_t const id = d[0] * pixel[1] + pixel[0];

    aov_.add_pixel(id, slot, value, weight);
}

void Sensor::add_AOV_atomic(int2 pixel, uint32_t slot, float3 const& value, float weight) {
    auto const d = dimensions();

    int32_t const id = d[0] * pixel[1] + pixel[0];

    aov_.add_pixel_atomic(id, slot, value, weight);
}

void Sensor::overwrite_AOV(int2 pixel, uint32_t slot, float3 const& value) {
    auto const d = dimensions();

    int32_t const id = d[0] * pixel[1] + pixel[0];

    aov_.overwrite_pixel(id, slot, value);
}

}  // namespace rendering::sensor
