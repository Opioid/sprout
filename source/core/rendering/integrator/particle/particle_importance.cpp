#include "particle_importance.hpp"
#include "base/atomic/atomic.hpp"
#include "base/math/distribution_1d.inl"
#include "base/math/vector2.inl"
#include "base/memory/array.inl"
#include "base/memory/buffer.hpp"
#include "base/thread/thread_pool.hpp"
#include "image/encoding/png/png_writer.hpp"
#include "scene/light/light.hpp"
#include "scene/scene.inl"
#include "scene/scene_worker.hpp"

namespace rendering::integrator::particle {

class Histogram {
  public:
    static int32_t constexpr Num_buckets = 16;

    Histogram(float max_value) : max_value_(max_value) {
        for (uint32_t i = 0; i < Num_buckets; ++i) {
            buckets_[i] = 0;
        }
    }

    void insert(float value) {
        uint32_t const i = uint32_t(std::lrint((value / max_value_) * float(Num_buckets - 1)));

        ++buckets_[i];
    }

    float max_of_lower(uint32_t lower, uint32_t total) const {
        if (lower == total) {
            return max_value_;
        }

        uint32_t count = total;

        int32_t i = Num_buckets - 1;
        for (; i >= 1; --i) {
            count -= buckets_[i];

            if (count < lower || 1 == i) {
                break;
            }
        }

        return i * (max_value_ / float(Num_buckets));
    }

  private:
    uint32_t buckets_[Num_buckets];

    float max_value_;
};

Importance::Importance() : importance_(new Weight[Dimensions * Dimensions]), valid_(false) {}

Importance::Importance(Importance&& other)
    : importance_(other.importance_),
      distribution_(std::move(other.distribution_)),
      weight_norm_(other.weight_norm_),
      valid_(other.valid_) {
    other.importance_ = nullptr;
}

Importance::~Importance() {
    delete[] importance_;
}

void Importance::clear(uint32_t num_expected_particles) {
    for (int32_t i = 0, len = Dimensions * Dimensions; i < len; ++i) {
        importance_[i] = {0.f, 0};
    }

    valid_ = false;

    weight_norm_ = 1.f / (num_expected_particles);
}

bool Importance::valid() const {
    return valid_;
}

void Importance::increment(float2 uv, float weight) {
    int32_t const x = std::min(int32_t(std::lrint(uv[0] * float(Dimensions - 1))), Dimensions - 1);
    int32_t const y = std::min(int32_t(std::lrint(uv[1] * float(Dimensions - 1))), Dimensions - 1);

    int32_t const id = y * Dimensions + x;

    weight *= weight_norm_;

    atomic::add_assign(importance_[id].w, weight);
    atomic::add_assign(importance_[id].c, 1);
}

Distribution_2D const& Importance::distribution() const {
    return distribution_;
}

float Importance::denormalization_factor() const {
    return float(Dimensions * Dimensions);
}

void Importance::prepare_sampling(uint32_t id, float* buffer, scene::Scene const& scene,
                                  Threads& threads) {
    if (valid()) {
        return;
    }

    float4 const cone = scene.light_cone(id);

    if (cone[3] < 0.5f) {
        return;
    }

    filter(buffer, threads);

    static int32_t constexpr N = Dimensions * Dimensions;

    float max = 0.f;
    for (int32_t i = 0; i < N; ++i) {
        max = std::max(buffer[i], max);
    }

    Histogram hist(max);

    for (int32_t i = 0; i < N; ++i) {
        hist.insert(buffer[i]);
    }

    max = hist.max_of_lower(uint32_t(0.9f * float(N)), N);

    if (0.f == max) {
        return;
    }

    std::string const name = "particle_importance_" + std::to_string(id) + ".png";

    image::encoding::png::Writer::write_heatmap(name, buffer, int2(Dimensions), max, threads);

    distribution_.allocate(Dimensions);

    threads.run_range(
        [this, buffer, max](uint32_t /*id*/, int32_t begin, int32_t end) noexcept {
            Distribution_1D* conditional = distribution_.conditional();

            auto weights = memory::Buffer<float>(Dimensions);

            float const im = 1.f / max;

            for (int32_t y = begin; y < end; ++y) {
                int32_t const row = y * Dimensions;

                for (int32_t x = 0; x < Dimensions; ++x) {
                    int32_t const i = row + x;

                    float const weight = std::min(buffer[i], max) * im;

                    weights[x] = weight;
                }

                conditional[y].init(weights.data(), Dimensions);
            }
        },
        0, Dimensions);

    distribution_.init();

    valid_ = true;
}

void Importance::filter(float* buffer, Threads& threads) const {
    static int32_t constexpr Kernel_radius = 1;

    threads.run_range(
        [this, buffer](uint32_t /*id*/, int32_t begin, int32_t end) noexcept {
            for (int32_t y = begin; y < end; ++y) {
                int32_t const row = y * Dimensions;

                for (int32_t x = 0; x < Dimensions; ++x) {
                    int32_t const i = row + x;

                    float max_value = 0.f;

                    for (int32_t ky = -Kernel_radius; ky <= Kernel_radius; ++ky) {
                        for (int32_t kx = -Kernel_radius; kx <= Kernel_radius; ++kx) {
                            int32_t const tx = x + kx;
                            int32_t const ty = y + ky;

                            if (tx >= 0 && tx < Dimensions && ty >= 0 && ty < Dimensions) {
                                int32_t const o = ty * Dimensions + tx;

                                Weight const w = importance_[o];

                                float const value = w.c > 0 ? w.w / float(w.c) : 0.f;

                                max_value = std::max(value, max_value);
                            }
                        }
                    }

                    buffer[i] = max_value;
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

void Importance_cache::clear(uint32_t num_expected_particles) {
    for (auto& importance : importances_) {
        importance.clear(num_expected_particles);
    }
}

void Importance_cache::set_training(bool training) {
    training_ = training;
}

void Importance_cache::prepare_sampling(scene::Scene const& scene, Threads& threads) {
    for (uint32_t i = 0; auto& importance : importances_) {
        importance.prepare_sampling(i, buffer_, scene, threads);

        ++i;
    }
}

void Importance_cache::increment(uint32_t light_id, float2 uv) {
    if (training_) {
        importances_[light_id].increment(uv, 1.f);
    }
}

void Importance_cache::increment(uint32_t light_id, float2 uv, Intersection const& isec,
                                 uint64_t time, float weight, Worker const& worker) {
    if (training_) {
        float4 const dd = worker.screenspace_differential(isec, time);

        float const w = weight / std::max(max_component(abs(dd)), 1.e-4f);

        importances_[light_id].increment(uv, w);
    }
}

Importance const& Importance_cache::importance(uint32_t light_id) const {
    return importances_[light_id];
}

}  // namespace rendering::integrator::particle
