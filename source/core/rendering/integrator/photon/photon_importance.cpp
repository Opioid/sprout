#include "photon_importance.hpp"
#include "base/atomic/atomic.hpp"
#include "base/math/vector2.inl"
#include "base/memory/align.hpp"
#include "image/encoding/png/png_writer.hpp"

namespace rendering::integrator::photon {

static uint32_t constexpr Dimensions = 256;

Importance::Importance() noexcept
    : dimensions_(Dimensions),
      importance_(memory::allocate_aligned<float>(Dimensions * Dimensions)),
      dimensions_back_(Dimensions - 1),
      dimensions_float_(float2(Dimensions)) {
    for (int32_t i = 0, len = dimensions_[i] * dimensions_[1]; i < len; ++i) {
        importance_[i] = 0.f;
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

    atomic::add_assign(importance_[id], 1.f);
}

void Importance::export_heatmap(std::string_view name) const noexcept {
    image::encoding::png::Writer::write_heatmap(name, importance_, dimensions_);
}

}  // namespace rendering::integrator::photon
