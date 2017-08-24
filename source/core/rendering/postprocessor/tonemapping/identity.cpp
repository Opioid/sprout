#include "identity.hpp"
#include "image/typed_image.inl"
#include "base/math/vector4.inl"

namespace rendering { namespace postprocessor { namespace tonemapping {

void Identity::apply(uint32_t /*id*/, uint32_t /*pass*/, int32_t begin, int32_t end,
					 const image::Float4& source, image::Float4& destination) {
	for (int32_t i = begin; i < end; ++i) {
		const float4& color = source.at(i);

		destination.at(i) = color;
	}
}

}}}
