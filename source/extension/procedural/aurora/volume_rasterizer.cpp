#include "volume_rasterizer.hpp"
#include "base/encoding/encoding.inl"
#include "base/math/vector3.inl"
#include "base/memory/align.hpp"
#include "base/spectrum/rgb.hpp"
#include "core/image/typed_image.inl"

namespace procedural::aurora {

Volume_rasterizer::Volume_rasterizer(int3 const& dimensions)
    : dimensions_(dimensions),
      area_(dimensions[0] * dimensions[1]),
      samples_(memory::allocate_aligned<float3>(dimensions[0] * dimensions[1] * dimensions[2])) {}

Volume_rasterizer::~Volume_rasterizer() {
    memory::free_aligned(samples_);
}

void Volume_rasterizer::resolve(image::Byte3& target) const {
    int32_t const len = dimensions_[0] * dimensions_[1] * dimensions_[2];

    for (int32_t i = 0; i < len; ++i) {
        float3 const color = spectrum::linear_RGB_to_sRGB(samples_[i]);
        target.store(i, encoding::float_to_unorm(color));
    }
}

void Volume_rasterizer::set_brush(float3 const& color) {
    brush_ = color;
}

void Volume_rasterizer::clear() {
    int32_t const len = dimensions_[0] * dimensions_[1] * dimensions_[2];

    for (int32_t i = 0; i < len; ++i) {
        samples_[i] = brush_;
    }
}

void Volume_rasterizer::splat(float3 const& position, float3 const& color) {
    const int3 c(position);

    if (c[0] < 0 || c[0] >= dimensions_[0] || c[1] < 0 || c[1] >= dimensions_[1] || c[2] < 0 ||
        c[2] >= dimensions_[2]) {
        return;
    }

    int32_t const i = c[2] * area_ + c[1] * dimensions_[0] + c[0];

    samples_[i] = color;
}

float3* Volume_rasterizer::data() {
    return samples_;
}

}  // namespace procedural::aurora
