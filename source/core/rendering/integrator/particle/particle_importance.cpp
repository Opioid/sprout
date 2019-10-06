#include "particle_importance.hpp"
#include "base/atomic/atomic.hpp"
#include "base/math/distribution/distribution_1d.inl"
#include "base/math/vector2.inl"
#include "base/memory/align.hpp"
#include "base/thread/thread_pool.hpp"
#include "image/encoding/png/png_writer.hpp"
#include "scene/light/light.hpp"
#include "scene/scene.inl"

namespace rendering::integrator::particle {

static int32_t constexpr Dimensions = 256;

Importance::Importance() noexcept
    : dimensions_(Dimensions),
      importance_(memory::allocate_aligned<float>(Dimensions * Dimensions)),
      dimensions_back_(Dimensions - 1),
      dimensions_float_(float2(Dimensions)) {
    for (int32_t i = 0, len = dimensions_[0] * dimensions_[1]; i < len; ++i) {
        importance_[i] = 0.f;
    }
}

Importance::~Importance() noexcept {
    memory::free_aligned(importance_);
}

void Importance::increment(float2 uv, float weight) noexcept {
    int32_t const x = std::min(int32_t(uv[0] * dimensions_float_[0]), dimensions_back_[0]);
    int32_t const y = std::min(int32_t(uv[1] * dimensions_float_[1]), dimensions_back_[1]);

    int32_t const id = y * dimensions_[0] + x;

    atomic::add_assign(importance_[id], weight);
}

Distribution_2D const& Importance::distribution() const noexcept {
    return distribution_;
}

float Importance::denormalization_factor() const noexcept {
    return float(Dimensions * Dimensions);
}

void Importance::export_heatmap(std::string_view name) const noexcept {
    image::encoding::png::Writer::write_heatmap(name, importance_, dimensions_);
}

void Importance::prepare_sampling(thread::Pool& pool) noexcept {
    //    return;

    if (!distribution_.empty()) {
        return;
    }

    distribution_.allocate(Dimensions);

    float max = 0.f;
    for (int32_t i = 0, len = Dimensions * Dimensions; i < len; ++i) {
        max = std::max(importance_[i], max);
    }

    pool.run_range(
        [this, max](uint32_t /*id*/, int32_t begin, int32_t end) {
            Distribution_2D::Distribution_impl* conditional = distribution_.conditional();

            auto weights = memory::Buffer<float>(Dimensions);

            for (int32_t y = begin; y < end; ++y) {
                int32_t const row = y * Dimensions;

                for (int32_t x = 0; x < Dimensions; ++x) {
                    int32_t const i = row + x;

                    float const weight = std::max(importance_[i] / max, 0.002f);

                    weights[x] = weight;
                }

                conditional[y].init(weights.data(), Dimensions);
            }
        },
        0, Dimensions);

    distribution_.init();
}

Importance_cache::Importance_cache() noexcept : num_importances_(0), importances_(nullptr) {}

Importance_cache::~Importance_cache() noexcept {
    memory::destroy_aligned(importances_, num_importances_);
}

void Importance_cache::init(scene::Scene const& scene) noexcept {
    num_importances_ = uint32_t(scene.lights().size());

    importances_ = memory::construct_array_aligned<Importance>(scene.lights().size());
}

void Importance_cache::set_eye_position(float3 const& eye) noexcept {
    eye_ = eye;
}

void Importance_cache::set_training(bool training) noexcept {
    training_ = training;
}

void Importance_cache::prepare_sampling(thread::Pool& pool) noexcept {
    importances_[1].prepare_sampling(pool);

    //        for (uint32_t i = 0, len = num_importances_; i < len; ++i) {
    //            importances_[i].prepare_sampling(pool);
    //        }
}

void Importance_cache::increment_importance(uint32_t light_id, float2 uv) noexcept {
    if (training_) {
        importances_[light_id].increment(uv, 1.f);
    }
}

void Importance_cache::increment_importance(uint32_t light_id, float2 uv,
                                            float3 const& p) noexcept {
    if (training_) {
        float const d = std::exp(std::max(distance(p, eye_), 1.f));

        importances_[light_id].increment(uv, 1.f / d);
    }
}

Importance const& Importance_cache::importance(uint32_t light_id) const noexcept {
    return importances_[light_id];
}

void Importance_cache::export_importances() const noexcept {
    for (uint32_t i = 0, len = num_importances_; i < len; ++i) {
        importances_[i].export_heatmap("particle_importance_" + std::to_string(i) + ".png");
    }
}

}  // namespace rendering::integrator::particle
