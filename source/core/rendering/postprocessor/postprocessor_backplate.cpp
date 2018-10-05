#include "postprocessor_backplate.hpp"
#include "base/math/vector4.inl"
#include "image/texture/texture.hpp"
#include "image/typed_image.inl"

namespace rendering::postprocessor {

Backplate::Backplate(Texture_ptr const& backplate) : Postprocessor(1), backplate_(backplate) {}

void Backplate::init(scene::camera::Camera const& /*camera*/, thread::Pool& /*pool*/) {}

bool Backplate::alpha_out(bool /*alpha_in*/) const {
    return false;
}

size_t Backplate::num_bytes() const {
    return sizeof(*this);
}

void Backplate::apply(uint32_t /*id*/, uint32_t /*pass*/, int32_t begin, int32_t end,
                      const image::Float4& source, image::Float4& destination) {
    for (int32_t i = begin; i < end; ++i) {
        float4 const foreground = source.load(i);

        float3 const background = backplate_->at_3(i);

        float const alpha = foreground[3];

        destination.store(i, float4(foreground.xyz() + (1.f - alpha) * background, 1.f));
    }
}

}  // namespace rendering::postprocessor
