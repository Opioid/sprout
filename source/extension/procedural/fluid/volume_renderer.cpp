#include "volume_renderer.hpp"
#include "base/encoding/encoding.inl"
#include "base/math/vector3.inl"
#include "base/memory/align.hpp"
#include "base/spectrum/rgb.hpp"
#include "core/image/typed_image.hpp"

namespace procedural::fluid {

Volume_renderer::Volume_renderer(int3 const& dimensions, uint32_t max_saturation) noexcept
    : dimensions_(dimensions),
      voxels_(memory::allocate_aligned<Type>(dimensions[0] * dimensions[1] * dimensions[2])),
      max_saturation_(static_cast<float>(max_saturation)) {}

Volume_renderer::~Volume_renderer() noexcept {
    memory::free_aligned(voxels_);
}

void Volume_renderer::resolve(image::Byte4& target) const noexcept {
    int32_t const len = dimensions_[0] * dimensions_[1] * dimensions_[2];

    for (int32_t i = 0; i < len; ++i) {
        float4 const value = float4(voxels_[i]);

        //        float4 const color = min(value, max_saturation_) / max_saturation_;

        //        float4 const srgb = spectrum::linear_to_gamma_sRGB(color);

        float3 const rgb   = value.xyz() / value[3];
        float const  alpha = std::min(value[3], max_saturation_) / max_saturation_;

        float4 const srgb = spectrum::linear_to_gamma_sRGB(float4(rgb, alpha));

        byte4 const result = encoding::float_to_unorm(srgb);

        target.store(i, result);
    }
}

void Volume_renderer::resolve(image::Float1& target) const noexcept {
    int32_t const len = dimensions_[0] * dimensions_[1] * dimensions_[2];

    for (int32_t i = 0; i < len; ++i) {
        float const value = 0.f;  // float(voxels_[i]);

        float const color = std::min(value, max_saturation_) / max_saturation_;

        target.store(i, color);
    }
}

void Volume_renderer::clear() noexcept {
    int32_t const len = dimensions_[0] * dimensions_[1] * dimensions_[2];

    for (int32_t i = 0; i < len; ++i) {
        voxels_[i] = Type(0.f);
    }
}

void Volume_renderer::splat(float3 const& uvw, Type const& value) noexcept {
    const int3 c(uvw * float3(dimensions_) + 0.5f);

    float constexpr weight = 1.f / (3.f * 3.f * 3.f);

    //  float constexpr weight = 1.f;

    splat(c + int3(-1, -1, -1), weight * value);
    splat(c + int3(0, -1, -1), weight * value);
    splat(c + int3(+1, -1, -1), weight * value);

    splat(c + int3(-1, 0, -1), weight * value);
    splat(c + int3(0, 0, -1), weight * value);
    splat(c + int3(+1, 0, -1), weight * value);

    splat(c + int3(-1, 1, -1), weight * value);
    splat(c + int3(0, 1, -1), weight * value);
    splat(c + int3(+1, 1, -1), weight * value);

    splat(c + int3(-1, -1, 0), weight * value);
    splat(c + int3(0, -1, 0), weight * value);
    splat(c + int3(+1, -1, 0), weight * value);

    splat(c + int3(-1, 0, 0), weight * value);
    splat(c + int3(0, 0, 0), weight * value);
    splat(c + int3(+1, 0, 0), weight * value);

    splat(c + int3(-1, 1, 0), weight * value);
    splat(c + int3(0, 1, 0), weight * value);
    splat(c + int3(+1, 1, 0), weight * value);

    splat(c + int3(-1, -1, 1), weight * value);
    splat(c + int3(0, -1, 1), weight * value);
    splat(c + int3(+1, -1, 1), weight * value);

    splat(c + int3(-1, 0, 1), weight * value);
    splat(c + int3(0, 0, 1), weight * value);
    splat(c + int3(+1, 0, 1), weight * value);

    splat(c + int3(-1, 1, 1), weight * value);
    splat(c + int3(0, 1, 1), weight * value);
    splat(c + int3(+1, 1, 1), weight * value);
}

void Volume_renderer::splat(int3 const& c, Type const& value) noexcept {
    if (c[0] < 0 || c[0] >= dimensions_[0] || c[1] < 0 || c[1] >= dimensions_[1] || c[2] < 0 ||
        c[2] >= dimensions_[2]) {
        return;
    }

    int32_t const i = (c[2] * dimensions_[1] + c[1]) * dimensions_[0] + c[0];

    voxels_[i] += value;
}

}  // namespace procedural::fluid
