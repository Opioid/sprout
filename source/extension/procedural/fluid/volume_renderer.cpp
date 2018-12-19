#include "volume_renderer.hpp"
#include "base/encoding/encoding.inl"
#include "base/math/vector3.inl"
#include "base/memory/align.hpp"
#include "base/spectrum/rgb.hpp"
#include "core/image/typed_image.hpp"

namespace procedural::fluid {

Volume_renderer::Volume_renderer(int3 const& dimensions, uint32_t max_saturation) noexcept
    : dimensions_(dimensions),
      voxels_(memory::allocate_aligned<float>(dimensions[0] * dimensions[1] * dimensions[2])),
      max_saturation_(static_cast<float>(max_saturation)) {}

Volume_renderer::~Volume_renderer() noexcept {
    memory::free_aligned(voxels_);
}

void Volume_renderer::resolve(image::Float1& target) const noexcept {
    int32_t const len = dimensions_[0] * dimensions_[1] * dimensions_[2];

    for (int32_t i = 0; i < len; ++i) {
        float const value = voxels_[i];

        float const color = std::min(value, max_saturation_) / max_saturation_;

        target.store(i, color);
    }
}

void Volume_renderer::clear() noexcept {
    int32_t const len = dimensions_[0] * dimensions_[1] * dimensions_[2];

    for (int32_t i = 0; i < len; ++i) {
        voxels_[i] = 0.f;
    }
}

void Volume_renderer::splat(float3 const& position, float value) noexcept {
    const int3 c(position);

    if (c[0] < 0 || c[0] >= dimensions_[0] || c[1] < 0 || c[1] >= dimensions_[1] || c[2] < 0 ||
        c[2] >= dimensions_[2]) {
        return;
    }

    int32_t const i = (c[2] * dimensions_[1] + c[1]) * dimensions_[0] + c[0];

    voxels_[i] += value;
}

}  // namespace procedural::fluid
