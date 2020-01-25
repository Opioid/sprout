#include "particle_importance.hpp"
#include "base/atomic/atomic.hpp"
#include "base/math/distribution/distribution_1d.inl"
#include "base/math/vector2.inl"
#include "base/memory/align.hpp"
#include "base/memory/array.inl"
#include "base/thread/thread_pool.hpp"
#include "image/encoding/png/png_writer.hpp"
#include "scene/light/light.hpp"
#include "scene/scene.inl"

namespace rendering::integrator::particle {

Importance::Importance() noexcept
    : importance_(memory::allocate_aligned<float>(Dimensions * Dimensions)) {
    for (int32_t i = 0, len = Dimensions * Dimensions; i < len; ++i) {
        importance_[i] = 0.f;
    }
}

Importance::~Importance() noexcept {
    memory::free_aligned(importance_);
}

void Importance::increment(float2 uv, float weight) noexcept {
    int32_t const x = std::min(int32_t(uv[0] * float(Dimensions - 1) + 0.5f), Dimensions - 1);
    int32_t const y = std::min(int32_t(uv[1] * float(Dimensions - 1) + 0.5f), Dimensions - 1);

    int32_t const id = y * Dimensions + x;

    atomic::add_assign(importance_[id], weight);
}

Distribution_2D const& Importance::distribution() const noexcept {
    return distribution_;
}

float Importance::denormalization_factor() const noexcept {
    return float(Dimensions * Dimensions);
}

void Importance::prepare_sampling(uint32_t id, float* buffer, thread::Pool& threads) noexcept {
    if (!distribution_.empty()) {
        return;
    }

    dilate(buffer);

    std::string const name = "particle_importance_" + std::to_string(id) + ".png";

    image::encoding::png::Writer::write_heatmap(name, buffer, int2(Dimensions));

    distribution_.allocate(Dimensions);

    float max = 0.f;
    for (int32_t i = 0, len = Dimensions * Dimensions; i < len; ++i) {
        max = std::max(buffer[i], max);
    }

    threads.run_range(
        [this, buffer, max](uint32_t /*id*/, int32_t begin, int32_t end) {
            Distribution_2D::Distribution_impl* conditional = distribution_.conditional();

            auto weights = memory::Buffer<float>(Dimensions);

            for (int32_t y = begin; y < end; ++y) {
                int32_t const row = y * Dimensions;

                for (int32_t x = 0; x < Dimensions; ++x) {
                    int32_t const i = row + x;

                    float const weight = std::max(buffer[i] / max, 0.01f);

                    weights[x] = weight;
                }

                conditional[y].init(weights.data(), Dimensions);
            }
        },
        0, Dimensions);

    distribution_.init();
}

void Importance::dilate(float* buffer) const noexcept {
    for (int32_t i = 0, len = Dimensions * Dimensions; i < len; ++i) {
        buffer[i] = 0.f;
    }

    static int32_t constexpr Kernel_radius = 4;

    for (int32_t y = 0; y < Dimensions; ++y) {
        int32_t const row = y * Dimensions;

        for (int32_t x = 0; x < Dimensions; ++x) {
            int32_t const i = row + x;

            float const value = importance_[i];

            for (int32_t ky = -Kernel_radius; ky <= Kernel_radius; ++ky) {
                for (int32_t kx = -Kernel_radius; kx <= Kernel_radius; ++kx) {
                    int32_t const tx = x + kx;
                    int32_t const ty = y + ky;

                    if (tx >= 0 && tx < Dimensions && ty >= 0 && ty < Dimensions) {
                        int32_t const o = ty * Dimensions + tx;

                        //    buffer[o] += value;

                        buffer[o] += (1.f / std::max(length(float2(kx, ky)), 1.f)) * value;
                    }
                }
            }
        }
    }
}

Importance_cache::Importance_cache() noexcept
    : buffer_(memory::allocate_aligned<float>(Importance::Dimensions * Importance::Dimensions)) {}

Importance_cache::~Importance_cache() noexcept {
    memory::free_aligned(buffer_);
}

void Importance_cache::init(scene::Scene const& scene) noexcept {
    importances_.resize(scene.num_lights());
}

void Importance_cache::set_eye_position(float3 const& eye) noexcept {
    eye_ = eye;
}

void Importance_cache::set_training(bool training) noexcept {
    training_ = training;
}

void Importance_cache::prepare_sampling(thread::Pool& threads) noexcept {
    // This entire ordeal is very hacky!
    // We need a proper way to select which light should have importances and which not.
    uint32_t const light = std::min(1u, uint32_t(importances_.size()) - 1);

    importances_[light].prepare_sampling(light, buffer_, threads);

    //        for (uint32_t i = 0, len = num_importances_; i < len; ++i) {
    //            importances_[i].prepare_sampling(i, threads);
    //        }
}

void Importance_cache::increment(uint32_t light_id, float2 uv) noexcept {
    if (training_) {
        importances_[light_id].increment(uv, 1.f);
    }
}

void Importance_cache::increment(uint32_t light_id, float2 uv, float3 const& p) noexcept {
    if (training_) {
        float const d = std::exp(std::max(distance(p, eye_), 1.f));

        importances_[light_id].increment(uv, 1.f / d);
    }
}

Importance const& Importance_cache::importance(uint32_t light_id) const noexcept {
    return importances_[light_id];
}

}  // namespace rendering::integrator::particle
