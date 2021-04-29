#include "postprocessor_backplate.hpp"
#include "base/math/vector4.inl"
#include "image/texture/texture.inl"
#include "image/typed_image.hpp"

namespace rendering::postprocessor {

Backplate::Backplate(image::texture::Turbotexture const& backplate)
    : Postprocessor(1), backplate_(backplate) {}

void Backplate::init(scene::camera::Camera const& /*camera*/, Threads& /*threads*/) {}

bool Backplate::alpha_out(bool /*alpha_in*/) const {
    return false;
}

void Backplate::apply(uint32_t /*id*/, uint32_t /*pass*/, int32_t begin, int32_t end,
                      image::Float4 const& source, image::Float4& destination, Scene const& scene) {
    int2 const d = backplate_.description(scene).dimensions().xy();

    for (int32_t i = begin; i < end; ++i) {
        float4 const foreground = source.at(i);

        int2 const c = source.coordinates_2(i);

        bool const contained = c[0] < d[0] && c[1] < d[1];

        float3 const background = contained ? backplate_.at_3(c[0], c[1], scene) : float3(0.f);

        float const alpha = foreground[3];

        destination.store(i, float4(foreground.xyz() + (1.f - alpha) * background, 1.f));
    }
}

}  // namespace rendering::postprocessor
