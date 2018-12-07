#include "identity.hpp"
#include "base/math/vector4.inl"
#include "image/typed_image.hpp"

namespace rendering {
namespace postprocessor {
namespace tonemapping {

void Identity::apply(uint32_t /*id*/, uint32_t /*pass*/, int32_t begin, int32_t end,
                     image::Float4 const& source, image::Float4& destination) {
    for (int32_t i = begin; i < end; ++i) {
        float4 const& color = source.at(i);

        destination.store(i, color);
    }
}

}  // namespace tonemapping
}  // namespace postprocessor
}  // namespace rendering
