#include "photon_importance.hpp"
#include "base/atomic/atomic.hpp"
#include "base/math/distribution/distribution_1d.inl"
#include "base/math/vector2.inl"
#include "base/memory/align.hpp"
#include "base/thread/thread_pool.hpp"
#include "image/encoding/png/png_writer.hpp"

namespace rendering::integrator::photon {

static int32_t constexpr Dimensions = 256;

Importance::Importance() noexcept
    : dimensions_(Dimensions),
      importance_(memory::allocate_aligned<uint32_t>(Dimensions * Dimensions)),
      dimensions_back_(Dimensions - 1),
      dimensions_float_(float2(Dimensions)) {
    for (int32_t i = 0, len = dimensions_[i] * dimensions_[1]; i < len; ++i) {
        importance_[i] = 0;
    }
}

Importance::~Importance() noexcept {
    memory::free_aligned(importance_);
}

void Importance::increment(float2 uv) noexcept {
    int32_t const x = std::min(static_cast<int32_t>(uv[0] * dimensions_float_[0]),
                               dimensions_back_[0]);
    int32_t const y = std::min(static_cast<int32_t>(uv[1] * dimensions_float_[1]),
                               dimensions_back_[1]);

    int32_t const id = y * dimensions_[0] + x;

    atomic::add_assign(importance_[id], 1);
}

Distribution_2D const& Importance::distribution() const noexcept {
    return distribution_;
}

float Importance::denormalization_factor() const noexcept {
    return static_cast<float>(Dimensions * Dimensions);
}

void Importance::export_heatmap(std::string_view name) const noexcept {
    image::encoding::png::Writer::write_heatmap(name, importance_, dimensions_);
}

void Importance::prepare_sampling(thread::Pool& pool) noexcept {
 //    return;

    if (!distribution_.empty()) {
        return;
    }

    Distribution_2D::Distribution_impl* conditional = distribution_.allocate(Dimensions);

    uint32_t maxi = 0;
    for (int32_t i = 0, len = Dimensions * Dimensions; i < len; ++i) {
        maxi = std::max(importance_[i], maxi);
    }

    uint32_t mini = maxi / 128 + 1;

    float const max = static_cast<float>(maxi);

    pool.run_range(
        [this, &conditional, max, mini](uint32_t /*id*/, int32_t begin, int32_t end) {
            float* weights = memory::allocate_aligned<float>(Dimensions);

            for (int32_t y = begin; y < end; ++y) {
                int32_t const row = y * Dimensions;

                for (int32_t x = 0; x < Dimensions; ++x) {
                    int32_t const i = row + x;

                    float const weight = static_cast<float>(std::max(importance_[i], mini)) / max;

                    weights[x] = weight;
                }

                conditional[y].init(weights, Dimensions);
            }

            memory::free_aligned(weights);
        },
        0, Dimensions);

    distribution_.init();
}

}  // namespace rendering::integrator::photon
