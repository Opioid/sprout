#include "postprocessor_glare.hpp"
#include "image/typed_image.inl"
#include "base/math/vector.inl"

namespace rendering { namespace postprocessor {

void Glare::init(const scene::camera::Camera& camera) {

}

void Glare::apply(int32_t begin, int32_t end,
				  const image::Image_float_4& source,
				  image::Image_float_4& destination) const {
	for (int32_t i = begin; i < end; ++i) {
		const auto& in = source.at(i);

		destination.at(i) = float4(in.z, in.y, in.x, in.w);
	}
}

}}
