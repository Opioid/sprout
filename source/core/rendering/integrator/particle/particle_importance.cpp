#include "particle_importance.hpp"
#include "base/atomic/atomic.hpp"
#include "base/math/distribution/distribution_1d.inl"
#include "base/math/vector2.inl"
#include "base/memory/array.inl"
#include "base/memory/buffer.hpp"
#include "base/thread/thread_pool.hpp"
#include "image/encoding/png/png_writer.hpp"
#include "scene/light/light.hpp"
#include "scene/scene.inl"

namespace rendering::integrator::particle {

Importance::Importance() : importance_(new Weight[Dimensions * Dimensions]) {
    for (int32_t i = 0, len = Dimensions * Dimensions; i < len; ++i) {
        importance_[i] = {0.f, 0};
    }
}

Importance::~Importance() {
    delete[] importance_;
}

void Importance::increment(float2 uv, float weight) {
    int32_t const x = std::min(int32_t(std::lrint(uv[0] * float(Dimensions - 1))), Dimensions - 1);
    int32_t const y = std::min(int32_t(std::lrint(uv[1] * float(Dimensions - 1))), Dimensions - 1);

    int32_t const id = y * Dimensions + x;

    atomic::add_assign(importance_[id].w, weight);
    atomic::add_assign(importance_[id].c, 1);
}

Distribution_2D const& Importance::distribution() const {
    return distribution_;
}

float Importance::denormalization_factor() const {
    return float(Dimensions * Dimensions);
}

void Importance::prepare_sampling(uint32_t id, float* buffer, Threads& threads) {
    if (!distribution_.empty()) {
        return;
    }

    filter(buffer, threads);

    std::string const name = "particle_importance_" + std::to_string(id) + ".png";

    image::encoding::png::Writer::write_heatmap(name, buffer, int2(Dimensions), threads);

    distribution_.allocate(Dimensions);

    float max = 0.f;
    for (int32_t i = 0, len = Dimensions * Dimensions; i < len; ++i) {
        max = std::max(buffer[i], max);
    }

    threads.run_range(
        [this, buffer, max](uint32_t /*id*/, int32_t begin, int32_t end) noexcept {
            Distribution_1D* conditional = distribution_.conditional();

            auto weights = memory::Buffer<float>(Dimensions);

            for (int32_t y = begin; y < end; ++y) {
                int32_t const row = y * Dimensions;

                for (int32_t x = 0; x < Dimensions; ++x) {
                    int32_t const i = row + x;

                    float const weight = buffer[i] / max;

                    weights[x] = weight;
                }

                conditional[y].init(weights.data(), Dimensions);
            }
        },
        0, Dimensions);

    distribution_.init();
}

void Importance::filter(float* buffer, Threads& threads) const {
    static int32_t constexpr Kernel_radius = 4;

    threads.run_range(
        [this, buffer](uint32_t /*id*/, int32_t begin, int32_t end) noexcept {
            for (int32_t y = begin; y < end; ++y) {
                int32_t const row = y * Dimensions;

                for (int32_t x = 0; x < Dimensions; ++x) {
                    int32_t const i = row + x;

                    float filtered   = 0.f;
                    float weight_sum = 0.f;

                    for (int32_t ky = -Kernel_radius; ky <= Kernel_radius; ++ky) {
                        for (int32_t kx = -Kernel_radius; kx <= Kernel_radius; ++kx) {
                            int32_t const tx = x + kx;
                            int32_t const ty = y + ky;

                            if (tx >= 0 && tx < Dimensions && ty >= 0 && ty < Dimensions) {
                                int32_t const o = ty * Dimensions + tx;

                                Weight const w = importance_[o];

                                float const value = w.c > 0 ? w.w / float(w.c) : 0.f;

                                float const weight = (1.f / (length(float2(kx, ky)) + 1.f));

                                filtered += value * weight;

                                weight_sum += weight;
                            }
                        }
                    }

                    buffer[i] = weight_sum > 0.f ? filtered / weight_sum : 0.f;
                }
            }
        },
        0, Dimensions);
}

Importance_cache::Importance_cache()
    : buffer_(new float[Importance::Dimensions * Importance::Dimensions]) {}

Importance_cache::~Importance_cache() {
    delete[] buffer_;
}

void Importance_cache::init(scene::Scene const& scene) {
    importances_.resize(scene.num_lights());
}

void Importance_cache::set_eye_position(float3_p eye) {
    eye_ = eye;
}

void Importance_cache::set_training(bool training) {
    training_ = training;
}

void Importance_cache::prepare_sampling(Threads& threads) {
    // This entire ordeal is very hacky!
    // We need a proper way to select which light should have importances and which not.
    uint32_t const light = std::min(1u, uint32_t(importances_.size()) - 1);

    importances_[light].prepare_sampling(light, buffer_, threads);

    //        for (uint32_t i = 0, len = num_importances_; i < len; ++i) {
    //            importances_[i].prepare_sampling(i, threads);
    //        }
}

void Importance_cache::increment(uint32_t light_id, float2 uv) {
    if (training_) {
        importances_[light_id].increment(uv, 1.f);
    }
}

void Importance_cache::increment(uint32_t light_id, float2 uv, float3_p p) {
    if (training_) {
        float const d = std::max(squared_distance(p, eye_), 1.f);

        importances_[light_id].increment(uv, 1.f / d);
    }
}

Importance const& Importance_cache::importance(uint32_t light_id) const {
    return importances_[light_id];
}

}  // namespace rendering::integrator::particle
